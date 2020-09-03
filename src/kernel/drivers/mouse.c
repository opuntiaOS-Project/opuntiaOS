#include <drivers/display.h>
#include <drivers/driver_manager.h>
#include <types.h>
#include <x86/idt.h>
#include <x86/port.h>
#include <utils/kassert.h>

// #define MOUSE_DRIVER_DEBUG

static int mouse_x = 0, mouse_y = 0;

void mouse_run();

static driver_desc_t _mouse_driver_info()
{
    driver_desc_t desc;
    desc.type = DRIVER_INPUT_SYSTEMS_DEVICE;
    desc.auto_start = true;
    desc.is_device_driver = false;
    desc.is_device_needed = false;
    desc.is_driver_needed = false;
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
    uint8_t is_mid_button_pressed = (resp >> 2) & 1;
    uint8_t is_right_button_pressed = (resp >> 1) & 1;
    uint8_t is_left_button_pressed = (resp >> 0) & 1;

    if (x_sign) {
        mouse_x += (int)xm;
    } else {
        mouse_x -= (int)xm;
    }

    if (y_sign) {
        mouse_y += (int)ym;
    } else {
        mouse_y -= (int)ym;
    }

#ifdef MOUSE_DRIVER_DEBUG
    if (mouse_x < 0) {
        kprintf("-%d ", -mouse_x);
    } else {
        kprintf("%d ", mouse_x);
    }
    if (my < 0) {
        kprintf("-%d\n", -mouse_y);
    } else {
        kprintf("%d\n", mouse_y);
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
}

bool mouse_install()
{
    driver_install(_mouse_driver_info());
    return true;
}