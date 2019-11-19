#include <fs/fat16/fat16.h>
#include <drivers/display.h>
#include <mem/malloc.h>
#include <utils/mem.h>

// Private

fat16_drive_desc_t _fat16_driver[VFS_MAX_DEV_COUNT];
uint8_t _fat16_drives_count = 0;

fs_desc_t _fat16_fs_desc();
void _fat16_read(vfs_device_t *t_vfs_dev, uint32_t t_addr, uint8_t *t_buf);
void _fat16_write(vfs_device_t *t_vfs_dev, uint32_t t_addr, uint8_t *t_buf, uint32_t t_buf_size);
uint8_t _fat16_load_fat(vfs_device_t *t_vfs_dev, fat16_drive_desc_t *t_drive);
uint8_t _fat16_register_drive(vfs_device_t *t_vfs_dev, uint8_t *t_buf);

// Utils
uint16_t _fat16_cluster_id_to_phys_addr(vfs_device_t *t_vfs_dev, uint16_t t_val);
void _fat16_element_to_vfs_element(fat16_element_t *t_fat_elem, vfs_element_t *t_vfs_elem);

// Fat Tools
void _fat16_edit_cluster(vfs_device_t *t_vfs_dev, uint16_t t_cluster_id, uint16_t t_value);
uint16_t _fat16_get_cluster_value(vfs_device_t *t_vfs_dev, uint16_t t_cluster_id);

uint16_t _fat16_find_free_cluster(vfs_device_t *t_vfs_dev);
void _fat16_take_cluster(vfs_device_t *t_vfs_dev, uint16_t t_cluster_id);
uint16_t _fat16_allocate_cluster(vfs_device_t *t_vfs_dev);
uint16_t _fat16_extend_cluster(vfs_device_t *t_vfs_dev, uint16_t t_cluster_id);
uint16_t _fat16_get_next_cluster(vfs_device_t *t_vfs_dev, uint16_t t_cluster_id);
void _fat16_set_cluster_as_last(vfs_device_t *t_vfs_dev, uint16_t t_cluster_id);
uint16_t _fat16_seek_cluters(vfs_device_t *t_vfs_dev, uint16_t t_cluster_id, uint16_t t_n);

void _fat16_free_sequence_of_clusters(vfs_device_t *t_vfs_dev, uint16_t t_cluster_id);

// Fat16 Element Tools
void _fat16_encode_element(fat16_element_t *t_element, uint8_t* t_buf);
fat16_element_t _fat16_decode_element(uint8_t* t_buf);
fat16_element_t _fat16_get_element(vfs_device_t *t_vfs_dev, uint8_t* t_buf, const char *t_dn, const char *t_de);
bool _fat16_save_element(vfs_device_t *t_vfs_dev, uint8_t* t_cluster_data, uint16_t t_cluster_id_to_write, fat16_element_t *t_element_to_save);

// Dir Tools
fat16_element_t _fat16_get_dir_by_path(vfs_device_t *t_vfs_dev, const char *t_path);

// File Tools
void _fat16_set_file_name(fat16_element_t *t_element, const char *t_n);
void _fat16_set_file_ext(fat16_element_t *t_element, const char *t_e);
void _fat16_set_attrs(fat16_element_t *t_element, uint16_t t_attrs);
void _fat16_set_start_cluster(fat16_element_t *t_element, uint16_t t_sc);

// Private Implementation

fs_desc_t _fat16_fs_desc() {
    fs_desc_t fat16;
    fat16.recognize = fat16_recognize;
    fat16.create_dir = fat16_create_dir;
    fat16.lookup_dir = fat16_lookup_dir;
    fat16.write_file = fat16_write_file;
    fat16.read_file = fat16_read_file;
    return fat16;
}

void _fat16_read(vfs_device_t *t_vfs_dev, uint32_t t_addr, uint8_t *t_buf) {
    // printf("DRIVER ID:"); printd(t_vfs_dev->dev.driver_id);
    void (*rd)(device_t *d, uint32_t s, uint8_t *r) =
        drivers[t_vfs_dev->dev.driver_id].driver_desc.functions[DRIVER_STORAGE_READ];
    rd(&t_vfs_dev->dev, t_addr, t_buf);
}

void _fat16_write(vfs_device_t *t_vfs_dev, uint32_t t_addr, uint8_t *t_buf, uint32_t t_buf_size) {
    void (*wr)(device_t *d, uint32_t s, uint8_t *r, uint32_t bs) =
        drivers[t_vfs_dev->dev.driver_id].driver_desc.functions[DRIVER_STORAGE_WRITE];
    wr(&t_vfs_dev->dev, t_addr, t_buf, t_buf_size);
}

uint8_t _fat16_load_fat(vfs_device_t *t_vfs_dev, fat16_drive_desc_t *t_drive) {
    // Todo check Fats if multiple
    uint8_t data[512];
    uint32_t fat_size_bytes = t_drive->sectors_per_fat * t_drive->bytes_per_sector;
    t_drive->fat_ptr = (uint8_t*)kmalloc(fat_size_bytes);
    for (uint16_t sector_id = 0; sector_id < t_drive->sectors_per_fat; sector_id++) {
        _fat16_read(t_vfs_dev, t_drive->start_of_fats + sector_id, data);
        for (uint16_t i_copy = 0; i_copy < t_drive->bytes_per_sector; i_copy++) {
            uint32_t offset = sector_id * t_drive->bytes_per_sector + i_copy;
            t_drive->fat_ptr[offset] = data[i_copy];
        }
    }
}

uint8_t _fat16_register_drive(vfs_device_t *t_vfs_dev, uint8_t *t_buf) {
    fat16_drive_desc_t new_drive;
    new_drive.bytes_per_sector = t_buf[0x0c] * 0x100 + t_buf[0x0b];
    new_drive.sectors_per_cluster = 1; // data[0x0d]
    new_drive.bytes_per_cluster = new_drive.bytes_per_sector * new_drive.sectors_per_cluster;
    new_drive.reserved_sectors = t_buf[0x0f] * 0x100 + t_buf[0x0e];
    new_drive.start_of_fats = new_drive.reserved_sectors;
    new_drive.number_of_fats = t_buf[0x10];
    new_drive.sectors_per_fat = t_buf[0x17] * 0x100 + t_buf[0x16];
    new_drive.root_entries = t_buf[0x12] * 0x100 + t_buf[0x11];
    new_drive.root_dir_start = new_drive.start_of_fats + new_drive.number_of_fats * new_drive.sectors_per_fat;
    new_drive.data_seg_start = new_drive.root_dir_start + (new_drive.root_entries * 32) / new_drive.bytes_per_sector;

    _fat16_load_fat(t_vfs_dev, &new_drive);

    _fat16_driver[_fat16_drives_count++] = new_drive;
    return _fat16_drives_count - 1;
}

// --------
// Utils
// --------

uint16_t _fat16_cluster_id_to_phys_addr(vfs_device_t *t_vfs_dev, uint16_t t_val) {
    fat16_drive_desc_t *drive_desc = &_fat16_driver[t_vfs_dev->translate_id];
    if (t_val == 0) {
        return drive_desc->root_dir_start;
    }
    return drive_desc->data_seg_start + t_val - 1;
}

void _fat16_element_to_vfs_element(fat16_element_t *t_fat_elem, vfs_element_t *t_vfs_elem) {
    memset(t_vfs_elem->filename, 0, VFS_MAX_FILENAME);
    memset(t_vfs_elem->filename_ext, 0, VFS_MAX_FILENAME_EXT);
    memccpy(t_vfs_elem->filename, t_fat_elem->filename, 0, FAT16_MAX_FILENAME);
    memccpy(t_vfs_elem->filename_ext, t_fat_elem->filename_ext, 0, FAT16_MAX_FILENAME_EXT);
    t_vfs_elem->attributes = t_fat_elem->attributes;
    t_vfs_elem->file_size = t_fat_elem->file_size;
}

// --------
// Fat Tools
// --------

uint16_t _fat16_find_free_cluster(vfs_device_t *t_vfs_dev) {
    fat16_drive_desc_t *drive_desc = &_fat16_driver[t_vfs_dev->translate_id];
    uint32_t fat_size_bytes = drive_desc->sectors_per_fat * drive_desc->bytes_per_sector;
    for (uint16_t sector = 0; sector < fat_size_bytes; sector+=2) {
        uint16_t result = (drive_desc->fat_ptr[sector+1] << 8) + drive_desc->fat_ptr[sector];
        if (result == 0) {
            return sector / 2 - 1;
        }
    }
    return 0;
}

void _fat16_edit_cluster(vfs_device_t *t_vfs_dev, uint16_t t_cluster_id, uint16_t t_value) {
    uint16_t rec_in_fat = 2 * (t_cluster_id + 1);
    fat16_drive_desc_t *drive_desc = &_fat16_driver[t_vfs_dev->translate_id];
    drive_desc->fat_ptr[rec_in_fat] = t_value % 0x100;
    drive_desc->fat_ptr[rec_in_fat+1] = (t_value >> 8) % 0x100;
}

uint16_t _fat16_get_cluster_value(vfs_device_t *t_vfs_dev, uint16_t t_cluster_id) {
    uint16_t rec_in_fat = 2 * (t_cluster_id + 1);
    fat16_drive_desc_t *drive_desc = &_fat16_driver[t_vfs_dev->translate_id];
    uint16_t high = drive_desc->fat_ptr[rec_in_fat + 1];
    uint16_t low = drive_desc->fat_ptr[rec_in_fat];
    return (high << 8) + low;
}

void _fat16_take_cluster(vfs_device_t *t_vfs_dev, uint16_t t_cluster_id) {
    _fat16_edit_cluster(t_vfs_dev, t_cluster_id, 0xffff);
}

uint16_t _fat16_allocate_cluster(vfs_device_t *t_vfs_dev) {
    uint16_t cluster_id = _fat16_find_free_cluster(t_vfs_dev);
    _fat16_take_cluster(t_vfs_dev, cluster_id);
    return cluster_id;
}

uint16_t _fat16_extend_cluster(vfs_device_t *t_vfs_dev, uint16_t t_cluster_id) {
    uint16_t new_block = _fat16_allocate_cluster(t_vfs_dev);
    _fat16_edit_cluster(t_vfs_dev, t_cluster_id, new_block);
    return new_block;
}


void _fat16_free_sequence_of_clusters(vfs_device_t *t_vfs_dev, uint16_t t_cluster_id) {
    uint16_t cur_c = t_cluster_id, nxt_c;
    while (cur_c != 0xffff) {
        nxt_c = _fat16_get_cluster_value(t_vfs_dev, cur_c);
        _fat16_edit_cluster(t_vfs_dev, cur_c, 0x0000);
        cur_c = nxt_c;
    }
}

uint16_t _fat16_get_next_cluster(vfs_device_t *t_vfs_dev, uint16_t t_cluster_id) {
    uint16_t nxt_cluster = _fat16_get_cluster_value(t_vfs_dev, t_cluster_id);
    if (nxt_cluster == 0xffff) {
        nxt_cluster = _fat16_extend_cluster(t_vfs_dev, t_cluster_id);
    }
    return nxt_cluster;
}

void _fat16_set_cluster_as_last(vfs_device_t *t_vfs_dev, uint16_t t_cluster_id) {
    uint16_t nxt_cluster = _fat16_get_cluster_value(t_vfs_dev, t_cluster_id);
    _fat16_free_sequence_of_clusters(t_vfs_dev, nxt_cluster);
    _fat16_edit_cluster(t_vfs_dev, t_cluster_id, 0xffff);
}

uint16_t _fat16_seek_cluters(vfs_device_t *t_vfs_dev, uint16_t t_cluster_id, uint16_t t_n) {
    for (uint16_t i = 0; i < t_n; i++) {
        t_cluster_id = _fat16_get_cluster_value(t_vfs_dev, t_cluster_id);
        if (t_cluster_id == 0xffff) {
            break;
        }
    }
    return t_cluster_id;
}

// --------
// Fat16 Element Tools
// --------

void _fat16_encode_element(fat16_element_t *t_element, uint8_t* t_buf) {
    memset(t_buf, 0x0, FAT16_MAX_FILENAME + FAT16_MAX_FILENAME_EXT);
    memccpy(t_buf, t_element->filename, 0, FAT16_MAX_FILENAME);
    memccpy((t_buf+0x08), t_element->filename_ext, 0, FAT16_MAX_FILENAME_EXT);
    t_buf[0x0b] = t_element->attributes;
    t_buf[0x1a] = t_element->start_cluster_id % 0x100;
    t_buf[0x1a+1] = t_element->start_cluster_id / 0x100;

    // encoding file_size
    t_buf[0x1c] = t_element->file_size % 0x100;
    t_buf[0x1c+1] = (t_element->file_size >>  8) % 0x100;
    t_buf[0x1c+2] = (t_element->file_size >> 16) % 0x100;
    t_buf[0x1c+3] = (t_element->file_size >> 24) % 0x100;
}

fat16_element_t _fat16_decode_element(uint8_t* t_buf) {
    fat16_element_t res;
    memset(res.filename, 0x0, FAT16_MAX_FILENAME);
    memset(res.filename_ext, 0x0, FAT16_MAX_FILENAME_EXT);
    memccpy(res.filename, t_buf, 0x0, FAT16_MAX_FILENAME);
    memccpy(res.filename_ext, (t_buf+0x08), 0x0, FAT16_MAX_FILENAME_EXT);
    res.attributes = t_buf[0x0b];
    res.start_cluster_id = t_buf[0x1b] * 0x100 + t_buf[0x1a];
    res.file_size = (t_buf[0x1c] +
                    (t_buf[0x1c+1] <<  8) +
                    (t_buf[0x1c+2] << 16) +
                    (t_buf[0x1c+3] << 24));
    return res;
}

fat16_element_t _fat16_get_element(vfs_device_t *t_vfs_dev, uint8_t* t_buf, const char *t_dn, const char *t_de) {
    fat16_drive_desc_t *drive_desc = &_fat16_driver[t_vfs_dev->translate_id];
    uint16_t file_offset, ext_offset;
    bool ok = true;
    for (uint16_t e_offset = 0; e_offset < drive_desc->bytes_per_cluster; e_offset++) {
        ok = true;
        file_offset = e_offset;
        ext_offset = e_offset + FAT16_MAX_FILENAME;
        for (uint8_t let = 0; t_dn[let] != 0
                        && let < FAT16_MAX_FILENAME; let++) {
            ok &= (t_buf[file_offset+let] == t_dn[let]);
        }
        for (uint8_t let = 0; t_de[let] != 0
                        && let < FAT16_MAX_FILENAME_EXT; let++) {
            ok &= (t_buf[ext_offset+let] == t_de[let]);
        }

        if (ok) {
            return _fat16_decode_element((t_buf+e_offset));
        }
    }

    fat16_element_t tmp;
    tmp.attributes = FAT16_ELEMENT_NULL;
    return tmp;
}

// if t_cluster_data
bool _fat16_save_element(vfs_device_t *t_vfs_dev, uint8_t* t_cluster_data,
        uint16_t t_cluster_id_to_write, fat16_element_t *t_element_to_save) {

    fat16_drive_desc_t *drive_desc = &_fat16_driver[t_vfs_dev->translate_id];
    uint16_t block_phys_addr = _fat16_cluster_id_to_phys_addr(t_vfs_dev, t_cluster_id_to_write);
    uint8_t cl_data[512];
    if (!t_cluster_data) {
        _fat16_read(t_vfs_dev, block_phys_addr, cl_data);
    } else {
        memcpy(cl_data, t_cluster_data, 512);
    }

    // finding spot for the new element
    bool found = false;
    uint16_t e_offset;
    for (e_offset = 0; e_offset < drive_desc->bytes_per_sector; e_offset+=32) {
        // means that spot is empty
        if (cl_data[e_offset] == 0x00 ||
            cl_data[e_offset] == FAT16_DELETED_SIGN) {
            found = true;
            break;
        }
    }

    if (!found) {
        return false;
    }

    uint8_t buf[32];
    _fat16_encode_element(t_element_to_save, buf);
    memcpy(cl_data+e_offset, buf, 32);
    _fat16_write(t_vfs_dev, block_phys_addr, cl_data, 512);
    return true;
}

// --------
// Dir Tools
// --------

fat16_element_t _fat16_get_dir_by_path(vfs_device_t *t_vfs_dev, const char *t_path) {
    uint32_t path_len = strlen(t_path);
    fat16_element_t dir;
    dir.start_cluster_id = FAT16_ELEMENT_ROOT_BLOCK_ID;
    dir.attributes = FAT16_ELEMENT_ROOT_FOLDER;

    uint8_t nxt = 0;
    char dir_name[FAT16_MAX_FILENAME];
    char dir_ext[FAT16_MAX_FILENAME_EXT];
    memset(dir_name, 0x0, FAT16_MAX_FILENAME);
    memset(dir_ext, 0x0, FAT16_MAX_FILENAME_EXT);

    uint8_t buf[512];
    uint16_t cur_cluster = FAT16_ELEMENT_ROOT_BLOCK_ID;

    for (uint32_t i = 1; i < path_len; i++) {
        if (t_path[i] == '/') {
            uint16_t phys_addr = _fat16_cluster_id_to_phys_addr(t_vfs_dev, cur_cluster);
            _fat16_read(t_vfs_dev, phys_addr, buf);
            dir = _fat16_get_element(t_vfs_dev, buf, dir_name, dir_ext);

            if (dir.attributes != FAT16_ELEMENT_ROOT_FOLDER
                && dir.attributes != FAT16_ELEMENT_FOLDER) {
                // means dir doens't exist
                dir.attributes = FAT16_ELEMENT_NULL;
                return dir;
            }

            cur_cluster = dir.start_cluster_id;
        }
    }

    return dir;
}

//-------
// File Tools
//-------

void _fat16_set_file_name(fat16_element_t *t_element, const char *t_n) {
    memset(t_element->filename, 0x20, FAT16_MAX_FILENAME);
    memccpy(t_element->filename, t_n, 0x0, FAT16_MAX_FILENAME);
}

void _fat16_set_file_ext(fat16_element_t *t_element, const char *t_e) {
    memset(t_element->filename_ext, 0x20, FAT16_MAX_FILENAME);
    memccpy(t_element->filename_ext, t_e, 0x0, FAT16_MAX_FILENAME);
}

void _fat16_set_attrs(fat16_element_t *t_element, uint16_t t_attrs) {
    t_element->attributes = t_attrs;
}

void _fat16_set_start_cluster(fat16_element_t *t_element, uint16_t t_sc) {
    t_element->start_cluster_id = t_sc;
}

void _fat16_set_file_size(fat16_element_t *t_element, uint16_t t_fs) {
    t_element->file_size = t_fs;
}

// Public

void fat16_install() {
    vfs_add_fs(_fat16_fs_desc());
}

// Recognize if it's fat16 drive and if it's fat16 drive, will registered that
// drive (Collect info, load fat into ram, etc.)
bool fat16_recognize(vfs_device_t *t_vfs_dev) {
    uint8_t res[512];
    _fat16_read(t_vfs_dev, 0, res);
    char fat16Signature[] = {'F', 'A', 'T', '1', '6', 0x20, 0x20, 0x20};
    bool isSignatureCorrect = true;
    for (uint16_t i = 0x36; i < 0x36 + 8; i++) {
        isSignatureCorrect &= (fat16Signature[i - 0x36] == res[i]);
    }
    if (isSignatureCorrect) {
        uint8_t translate_id = _fat16_register_drive(t_vfs_dev, res);
        t_vfs_dev->translate_id = translate_id;
    }
    return isSignatureCorrect;
}

// --------
// Dir
// --------

bool fat16_create_dir(vfs_device_t *t_vfs_dev, const char *t_path, const char *t_dir_name) {
    fat16_element_t dir_to_save = _fat16_get_dir_by_path(t_vfs_dev, t_path);
    if (dir_to_save.attributes != FAT16_ELEMENT_FOLDER &&
        dir_to_save.attributes != FAT16_ELEMENT_ROOT_FOLDER) {
        return false;
    }

    fat16_element_t new_folder;
    new_folder.attributes = FAT16_ELEMENT_FOLDER;
    memset(new_folder.filename, 0x20, 8);
    memccpy(new_folder.filename, t_dir_name, 0, 8);
    memset(new_folder.filename_ext, 0x20, 3);

    new_folder.start_cluster_id = _fat16_find_free_cluster(t_vfs_dev);
    _fat16_take_cluster(t_vfs_dev, new_folder.start_cluster_id);

    return _fat16_save_element(t_vfs_dev, 0, dir_to_save.start_cluster_id, &new_folder);
}

uint32_t fat16_lookup_dir(vfs_device_t *t_vfs_dev, const char *t_path, vfs_element_t* t_buf) {
    fat16_drive_desc_t *drive_desc = &_fat16_driver[t_vfs_dev->translate_id];
    fat16_element_t dir_to_save = _fat16_get_dir_by_path(t_vfs_dev, t_path);
    uint16_t block_phyz_addr = _fat16_cluster_id_to_phys_addr(t_vfs_dev, dir_to_save.start_cluster_id);
    uint8_t cl_data[512];
    _fat16_read(t_vfs_dev, block_phyz_addr, cl_data);

    uint8_t added = 0;
    fat16_element_t e_fat_tmp;
    vfs_element_t e_vfs_tmp;
    for (uint16_t e_offset = 0; e_offset < drive_desc->bytes_per_sector; e_offset += 32) {
        if (cl_data[e_offset] != 0x00 &&
            cl_data[e_offset] != FAT16_DELETED_SIGN) {
            e_fat_tmp = _fat16_decode_element(cl_data+e_offset);
            _fat16_element_to_vfs_element(&e_fat_tmp, &e_vfs_tmp);
            t_buf[added++] = e_vfs_tmp;
        }
    }
    return added;
}

// Files

bool fat16_write_file(vfs_device_t *t_vfs_dev, const char *t_path, const char *t_file_name, const char *t_file_ext, const uint8_t *t_data, uint32_t t_size) {
    fat16_drive_desc_t *drive_desc = &_fat16_driver[t_vfs_dev->translate_id];
    fat16_element_t dir_to_save = _fat16_get_dir_by_path(t_vfs_dev, t_path);
    if (dir_to_save.attributes != FAT16_ELEMENT_FOLDER &&
        dir_to_save.attributes != FAT16_ELEMENT_ROOT_FOLDER) {
        return false;
    }

    uint8_t dir_buf[512];
    uint32_t dir_phys_addr = _fat16_cluster_id_to_phys_addr(t_vfs_dev, dir_to_save.start_cluster_id);
    _fat16_read(t_vfs_dev, dir_phys_addr, dir_buf);

    fat16_element_t file = _fat16_get_element(t_vfs_dev, dir_buf, t_file_name, t_file_ext);
    bool is_file_new = false;
    if (file.attributes == FAT16_ELEMENT_NULL) {
        // no such element, creating new
        _fat16_set_file_name(&file, t_file_name);
        _fat16_set_file_ext(&file, t_file_ext);
        _fat16_set_attrs(&file, 0x01);
        _fat16_set_start_cluster(&file, _fat16_allocate_cluster(t_vfs_dev));
        is_file_new = true;
    }
    _fat16_set_file_size(&file, t_size);

    uint16_t save_to_cluster = file.start_cluster_id;
    uint16_t last_edited_cluster = 0;
    uint16_t data_bytes_per_cluster = drive_desc->bytes_per_cluster - 2;
    uint8_t cluster_data[512];
    memset(cluster_data, 0, sizeof(cluster_data));

    for (uint16_t read_data = 0; read_data < t_size; read_data += data_bytes_per_cluster) {
        bool is_cluster_last = read_data + data_bytes_per_cluster >= t_size;
        uint16_t nxt_cluster = is_cluster_last ? 0xffff : _fat16_get_next_cluster(t_vfs_dev, save_to_cluster);
        uint16_t copy_size = is_cluster_last ? t_size-read_data : data_bytes_per_cluster;
        memcpy(cluster_data, t_data + read_data, copy_size);
        cluster_data[data_bytes_per_cluster] = nxt_cluster % 0x100;
        cluster_data[data_bytes_per_cluster+1] = (nxt_cluster >> 8) % 0x100;

        uint16_t cluster_plys_addr = _fat16_cluster_id_to_phys_addr(t_vfs_dev, save_to_cluster);
        _fat16_write(t_vfs_dev, cluster_plys_addr, cluster_data, 512);

        last_edited_cluster = save_to_cluster;
        save_to_cluster = nxt_cluster;
    }

    if (is_file_new) {
        _fat16_save_element(t_vfs_dev, dir_buf, dir_to_save.start_cluster_id, &file);
    } else {
        _fat16_set_cluster_as_last(t_vfs_dev, last_edited_cluster);
    }

    return true;
}


uint8_t* fat16_read_file(vfs_device_t *t_vfs_dev, const char *t_path, const char *t_file_name, const char *t_file_ext, uint16_t t_offset, int16_t t_len) {
    fat16_drive_desc_t *drive_desc = &_fat16_driver[t_vfs_dev->translate_id];
    fat16_element_t dir_to_save = _fat16_get_dir_by_path(t_vfs_dev, t_path);
    if (dir_to_save.attributes != FAT16_ELEMENT_FOLDER &&
        dir_to_save.attributes != FAT16_ELEMENT_ROOT_FOLDER) {
        return false;
    }

    uint8_t dir_buf[512];
    uint32_t dir_phys_addr = _fat16_cluster_id_to_phys_addr(t_vfs_dev, dir_to_save.start_cluster_id);
    _fat16_read(t_vfs_dev, dir_phys_addr, dir_buf);

    fat16_element_t file = _fat16_get_element(t_vfs_dev, dir_buf, t_file_name, t_file_ext);

    if (file.attributes == FAT16_ELEMENT_NULL) {
        // means no such file
        return 0;
    }

    if (t_len == -1) {
        t_len = file.file_size - t_offset;
    }
    
    if (t_offset + t_len > file.file_size) {
        return 0;
    }

    uint8_t c_buf[512];
    uint8_t *r_buf = (uint8_t*)kmalloc(t_len + 1);
    uint16_t r_ind = 0;

    uint16_t data_bytes = drive_desc->bytes_per_cluster-2;
    uint16_t seek = t_offset / data_bytes;
    uint16_t offset = t_offset % data_bytes;
    uint16_t nxt_cluster = _fat16_seek_cluters(t_vfs_dev, file.start_cluster_id, seek);    

    while (nxt_cluster != 0xffff) {
        uint16_t phys_addr = _fat16_cluster_id_to_phys_addr(t_vfs_dev, nxt_cluster);
        _fat16_read(t_vfs_dev, phys_addr, c_buf);

        for (uint16_t c_byte = offset; c_buf[c_byte] != 0 && r_ind < t_len && c_byte < data_bytes; c_byte++) {
            r_buf[r_ind++] = c_buf[c_byte];
        }

        nxt_cluster = (uint16_t)(c_buf[data_bytes+1] << 8) + (uint16_t)(c_buf[data_bytes+0]);
        offset = 0;
    }
    r_buf[r_ind] = '\0';
    return r_buf;
}