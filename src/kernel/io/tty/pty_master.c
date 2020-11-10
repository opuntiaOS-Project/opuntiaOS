#include <errno.h>
#include <fs/vfs.h>
#include <io/tty/pty_master.h>
#include <io/tty/pty_slave.h>
#include <log.h>
#include <utils/kassert.h>

/**
 * Since pty masters are virtual files and don't present on real hd,
 * we use an array of dentries. This dentries won't be present in the
 * list of dentries (dentry.c), so other threads con't access it.
 */
pty_master_entry_t pty_masters[PTYS_COUNT];

bool pty_master_can_read(dentry_t* dentry, uint32_t start);
bool pty_master_can_write(dentry_t* dentry, uint32_t start);
int pty_master_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len);
int pty_master_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len);

static file_ops_t pty_master_ops = {
    pty_master_can_read,
    pty_master_can_write,
    pty_master_read,
    pty_master_write,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

static pty_master_entry_t* _ptm_get(dentry_t* dentry)
{
    for (int i = 0; i < PTYS_COUNT; i++) {
        if (dentry == &pty_masters[i].dentry) {
            return &pty_masters[i];
        }
    }
    return 0;
}

bool pty_master_can_read(dentry_t* dentry, uint32_t start)
{
    pty_master_entry_t* ptm = _ptm_get(dentry);
    ASSERT(ptm);
    return ringbuffer_space_to_read(&ptm->buffer) >= 1;
}

bool pty_master_can_write(dentry_t* dentry, uint32_t start)
{
    pty_master_entry_t* ptm = _ptm_get(dentry);
    ASSERT(ptm);
    return ringbuffer_space_to_write(&ptm->pts->buffer) >= 0;
}

int pty_master_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    pty_master_entry_t* ptm = _ptm_get(dentry);
    ASSERT(ptm);
    uint32_t leno = ringbuffer_space_to_read(&ptm->buffer);
    if (leno > len) {
        leno = len;
    }
    int res = ringbuffer_read(&ptm->buffer, buf, leno);
    return leno;
}

int pty_master_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    pty_master_entry_t* ptm = _ptm_get(dentry);
    ASSERT(ptm);
    ringbuffer_write(&ptm->pts->buffer, buf, len);
    return len;
}

int pty_master_alloc(file_descriptor_t* fd)
{
    pty_master_entry_t* ptm = 0;
    for (int i = 0; i < PTYS_COUNT; i++) {
        if (pty_masters[i].dentry.inode_indx == 0) {
            /* According to dentry.c when inode_indx==0, dentry is free */
            ptm = &pty_masters[i];
            ptm->dentry.inode_indx = i + 1;
            break;
        }
    }

    if (!ptm) {
        return -EBUSY;
    }

    /* VFS will jump right into functions from ops starts */
    // d->ops = &pty_master_ops;

    ptm->dentry.flags = 0;
    dentry_set_flag(&ptm->dentry, DENTRY_PRIVATE);
    fd->dentry = &ptm->dentry;
    fd->ops = &pty_master_ops;
    fd->mapped_times = 0;
    fd->flags = 0;
    fd->offset = 0;
    fd->type = FD_TYPE_FILE;

    pty_slave_create(ptm->dentry.inode_indx, ptm);
    ptm->buffer = ringbuffer_create_std();

    return 0;
}