#include <algo/ringbuffer.h>
#include <drivers/display.h>
#include <drivers/driver_manager.h>
#include <drivers/mouse.h>
#include <fs/devfs/devfs.h>
#include <log.h>
#include <types.h>
#include <utils/kassert.h>
#include <x86/idt.h>
#include <x86/port.h>

// #define MOUSE_DRIVER_DEBUG

static ringbuffer_t mouse_buffer;

void mouse_run();

static bool _mouse_can_read(dentry_t* dentry, uint32_t start)
{
    return ringbuffer_space_to_read(&mouse_buffer) >= 1;
}

static int _mouse_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    uint32_t leno = ringbuffer_space_to_read(&mouse_buffer);
    if (leno > len) {
        leno = len;
    }
    int res = ringbuffer_read(&mouse_buffer, buf, leno);
    return leno;
}

static void _mouse_recieve_notification(uint32_t msg, uint32_t param)
{
    if (msg == DM_NOTIFICATION_DEVFS_READY) {
        dentry_t* mp;
        if (vfs_resolve_path("/dev", &mp) < 0) {
            kpanic("Can't init mouse in /dev");
        }

        file_ops_t fops;
        fops.can_read = _mouse_can_read;
        fops.can_write = 0;
        fops.read = _mouse_read;
        fops.write = 0;
        fops.ioctl = 0;
        fops.mmap = 0;
        devfs_inode_t* res = devfs_register(mp, "mouse", 5, 0, &fops);

        dentry_put(mp);
    }
}

static driver_desc_t _mouse_driver_info()
{
    driver_desc_t desc;
    desc.type = DRIVER_INPUT_SYSTEMS_DEVICE;
    desc.auto_start = true;
    desc.is_device_driver = false;
    desc.is_device_needed = false;
    desc.is_driver_needed = false;
    desc.functions[DRIVER_NOTIFICATION] = _mouse_recieve_notification;
    desc.functions[DRIVER_INPUT_SYSTEMS_ADD_DEVICE] = mouse_run;
    desc.functions[DRIVER_INPUT_SYSTEMS_GET_LAST_KEY] = 0;
    desc.functions[DRIVER_INPUT_SYSTEMS_DISCARD_LAST_KEY] = 0;
    desc.pci_serve_class = 0xff;
    desc.pci_serve_subclass = 0xff;
    desc.pci_serve_vendor_id = 0x00;
    desc.pci_serve_device_id = 0x00;
    return desc;
}

static inline void _mouse_wait_in()
{
    while (port_8bit_in(0x64) & 1 == 0) { }
}

static inline void _mouse_wait_out()
{
    while (port_8bit_in(0x64) & 2 == 0) { }
}

static inline uint8_t _mouse_wait_then_write(uint16_t port, uint8_t data)
{
    _mouse_wait_out();
    port_8bit_out(port, data);
}

static inline uint8_t _mouse_wait_then_read(uint16_t port)
{
    _mouse_wait_in();
    return port_8bit_in(port);
}

static inline void _mouse_send_cmd(uint8_t cmd)
{
    _mouse_wait_then_write(0x64, 0xD4);
    _mouse_wait_then_write(0x60, cmd);
    ASSERT(_mouse_wait_then_read(0x60) == 0xfa);
}

static inline void _mouse_enable_aux()
{
    _mouse_wait_then_write(0x64, 0x20);
    uint8_t res = _mouse_wait_then_read(0x60);
    res |= 0b10;
    res &= 0b11011111;
    _mouse_wait_then_write(0x64, 0x60);
    _mouse_wait_then_write(0x60, res);
}

void mouse_handler()
{
    uint8_t resp = port_8bit_in(0x60);
    uint8_t xm = port_8bit_in(0x60);
    uint8_t ym = port_8bit_in(0x60);

    uint8_t y_overflow = (resp >> 7) & 1;
    uint8_t x_overflow = (resp >> 6) & 1;
    uint8_t y_sign = (resp >> 5) & 1;
    uint8_t x_sign = (resp >> 4) & 1;

    mouse_packet_t packet;
    packet.x_offset = xm;
    packet.y_offset = ym;
    packet.button_states = resp & 0b111;

    if (packet.x_offset && x_sign) {
        packet.x_offset -= 0x100;
    }
    if (packet.y_offset && y_sign) {
        packet.y_offset -= 0x100;
    }
    if (x_overflow || y_overflow) {
        packet.x_offset = 0;
        packet.y_offset = 0;
    }

    ringbuffer_write(&mouse_buffer, (uint8_t*)&packet, sizeof(mouse_packet_t));

#ifdef MOUSE_DRIVER_DEBUG
    if (packet.x_offset < 0) {
        log("-%d ", -packet.x_offset);
    } else {
        log("%d ", packet.x_offset);
    }
    if (packet.y_offset < 0) {
        log("-%d\n", -packet.y_offset);
    } else {
        log("%d\n", packet.y_offset);
    }
#endif /* MOUSE_DRIVER_DEBUG */
}

void mouse_run()
{
    _mouse_wait_then_write(0x64, 0xa8);
    _mouse_enable_aux();
    _mouse_send_cmd(0xF6);
    _mouse_send_cmd(0xF4);
    set_irq_handler(IRQ12, mouse_handler);

    mouse_buffer = ringbuffer_create_std();
}

bool mouse_install()
{
    driver_install(_mouse_driver_info());
    return true;
}