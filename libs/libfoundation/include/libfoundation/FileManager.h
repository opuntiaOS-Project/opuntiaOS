/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <dirent.h>
#include <string>
#include <unistd.h>
#include <vector>

namespace LFoundation {

class FileManager {
public:
    FileManager() = default;
    ~FileManager() = default;

    template <typename Callback>
    int foreach_object(const std::string& path, Callback callback) const
    {
        const size_t temporal_buffer_size = 1024;
        char* temporal_buffer = new char[temporal_buffer_size];

        struct linux_dirent {
            uint32_t inode;
            uint16_t rec_len;
            uint8_t name_len;
            uint8_t file_type;
            char* name;
        } * d;

        int fd = open(path.c_str(), O_RDONLY | O_DIRECTORY);
        if (fd < 0) {
            return -1;
        }

        for (;;) {
            int nread = getdents(fd, temporal_buffer, temporal_buffer_size);
            if (nread == 0) {
                break;
            }

            for (int bpos = 0; bpos < nread;) {
                d = (struct linux_dirent*)(temporal_buffer + bpos);
                if (((char*)&d->name)[0] != '.') {
                    callback((char*)&d->name);
                }
                bpos += d->rec_len;
            }
        }

        close(fd);
        delete[] temporal_buffer;
        return 0;
    }

private:
};

} // namespace LFoundation