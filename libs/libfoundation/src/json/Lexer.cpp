/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libfoundation/json/Lexer.h>
#include <unistd.h>

namespace LFoundation::Json {

int Lexer::set_file(const std::string& filepath)
{
    char tmpbuf[1024];
    m_pointer = 0;

    int fd = open(filepath.c_str(), O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    ssize_t read_cnt;
    while ((read_cnt = read(fd, tmpbuf, sizeof(tmpbuf)))) {
        if (read_cnt <= 0) {
            return -2;
        }
        size_t buf_size = m_text_data.size();
        m_text_data.resize(buf_size + read_cnt);
        memcpy((uint8_t*)&m_text_data.data()[buf_size], (uint8_t*)tmpbuf, read_cnt);
        if (read_cnt < sizeof(tmpbuf)) {
            break;
        }
    }

    if (m_text_data.empty()) {
        return -3;
    }

    m_text_data.push_back('\0');
    close(fd);
    return 0;
}

}