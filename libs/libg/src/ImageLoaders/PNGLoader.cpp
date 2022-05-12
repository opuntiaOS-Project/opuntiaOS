/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstring>
#include <fcntl.h>
#include <libfoundation/Logger.h>
#include <libfoundation/compress/puff.h>
#include <libg/ImageLoaders/PNGLoader.h>
#include <memory>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

// #define PNGLOADER_DEGUG

namespace LG {
namespace PNG {

    constexpr int png_header_size = 8;
    static uint8_t png_header[] = { 137, 80, 78, 71, 13, 10, 26, 10 };

    PixelBitmap PNGLoader::load_from_file(const std::string& path)
    {
        int fd = open(path.c_str(), O_RDONLY);
        if (fd < 0) {
            Logger::debug << "PNGLoader: cant open" << std::endl;
            return PixelBitmap();
        }

        stat_t stat;
        fstat(fd, &stat);

        uint8_t* ptr = (uint8_t*)mmap(NULL, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        PixelBitmap bitmap = load_from_mem(ptr);

        munmap(ptr, stat.st_size);
        close(fd);
        return bitmap;
    }

    bool PNGLoader::check_header(const uint8_t* ptr) const
    {
        return memcmp(ptr, (uint8_t*)png_header, png_header_size) == 0;
    }

    void PNGLoader::read_IHDR(ChunkHeader& header, PixelBitmap& bitmap)
    {
        streamer().read(m_ihdr_chunk.width);
        streamer().read(m_ihdr_chunk.height);
        streamer().read(m_ihdr_chunk.depth);
        streamer().read(m_ihdr_chunk.color_type);
        streamer().read(m_ihdr_chunk.compression_method);
        streamer().read(m_ihdr_chunk.filter_method);
        streamer().read(m_ihdr_chunk.interlace_method);

        bitmap.resize(m_ihdr_chunk.width, m_ihdr_chunk.height);

#ifdef PNGLOADER_DEGUG
        Logger::debug << "IHDR: " << m_ihdr_chunk.width << " " << m_ihdr_chunk.depth << " " << m_ihdr_chunk.compression_method << " " << m_ihdr_chunk.filter_method << " " << m_ihdr_chunk.color_type << std::endl;
#endif
    }

    void PNGLoader::read_TEXT(ChunkHeader& header, PixelBitmap& bitmap)
    {
        streamer().skip(header.len);
    }

    void PNGLoader::read_PHYS(ChunkHeader& header, PixelBitmap& bitmap)
    {
        streamer().skip(header.len);
    }

    void PNGLoader::read_ORNT(ChunkHeader& header, PixelBitmap& bitmap)
    {
        streamer().skip(header.len);
    }

    void PNGLoader::read_gAMA(ChunkHeader& header, PixelBitmap& bitmap)
    {
        streamer().skip(header.len);
    }

    // TODO: Currently support only comprssion type 0
    void PNGLoader::read_IDAT(ChunkHeader& header, PixelBitmap& bitmap)
    {
        size_t buf_size = m_compressed_data.size();
        m_compressed_data.resize(buf_size + header.len);
        memcpy(&m_compressed_data.data()[buf_size], streamer().ptr(), header.len);
        streamer().skip(header.len);
    }

    void PNGLoader::process_compressed_data(PixelBitmap& bitmap)
    {
        size_t datalen = m_compressed_data.size();
        size_t destlen = 0;
        int ret = puff(0, &destlen, m_compressed_data.data() + 2, &datalen);
        uint8_t* unzipped_data = (uint8_t*)malloc(destlen);
        puff(unzipped_data, &destlen, m_compressed_data.data() + 2, &datalen);
        DataStreamer local_streamer(unzipped_data);
        m_scanline_keeper.init(unzipped_data);

        if (m_ihdr_chunk.color_type == 2) {
            m_scanline_keeper.set_color_length(3);
            if (m_ihdr_chunk.depth == 8) {
                for (int i = 0; i < m_ihdr_chunk.height; i++) {
                    uint8_t scanline_filter;
                    local_streamer.read(scanline_filter);

                    if (scanline_filter > 4) {
                        Logger::debug << "Invalid PNG filter: " << scanline_filter << std::endl;
                        continue;
                    }

                    size_t len_of_scanline = (m_scanline_keeper.color_length() * m_ihdr_chunk.width * m_ihdr_chunk.depth + 7) / 8;
                    m_scanline_keeper.add(Scanline(scanline_filter, local_streamer.ptr()));
                    local_streamer.skip(len_of_scanline);
                }
            }
        } else if (m_ihdr_chunk.color_type == 6) {
            m_scanline_keeper.set_color_length(4);
            if (m_ihdr_chunk.depth == 8) {
                for (int i = 0; i < m_ihdr_chunk.height; i++) {
                    uint8_t scanline_filter;
                    local_streamer.read(scanline_filter);

                    if (scanline_filter > 4) {
                        Logger::debug << "Invalid PNG filter: " << scanline_filter << std::endl;
                        continue;
                    }

                    size_t len_of_scanline = (m_scanline_keeper.color_length() * m_ihdr_chunk.width * m_ihdr_chunk.depth + 7) / 8;
                    m_scanline_keeper.add(Scanline(scanline_filter, local_streamer.ptr()));
                    local_streamer.skip(len_of_scanline);
                }
            }
        }

        unfilter_scanlines();
        copy_scanlines_to_bitmap(bitmap);
        m_scanline_keeper.invalidate();
    }

    uint8_t PNGLoader::paeth_predictor(int a, int b, int c)
    {
        int p = a + b - c;
        int pa = abs(p - a);
        int pb = abs(p - b);
        int pc = abs(p - c);
        if (pa <= pb && pa <= pc)
            return a;
        if (pb <= pc)
            return b;
        return c;
    }

    void PNGLoader::unfilter_scanlines()
    {
        for (int i = 0; i < m_ihdr_chunk.height; i++) {
            if (m_scanline_keeper.scanlines()[i].filter_type() == 1) { // Sub
                int width = m_ihdr_chunk.width * m_scanline_keeper.color_length();
                for (int j = 0; j < width; j++) {
                    int prev = 0;
                    if (j >= m_scanline_keeper.color_length()) {
                        prev = m_scanline_keeper.scanlines()[i].data()[j - m_scanline_keeper.color_length()];
                    }
                    m_scanline_keeper.scanlines()[i].data()[j] += prev;
                }
            } else if (m_scanline_keeper.scanlines()[i].filter_type() == 2) { // Up
                int width = m_ihdr_chunk.width * m_scanline_keeper.color_length();
                for (int j = 0; j < width; j++) {
                    int prev = 0;
                    if (i > 0) {
                        prev = m_scanline_keeper.scanlines()[i - 1].data()[j];
                    }
                    m_scanline_keeper.scanlines()[i].data()[j] += prev;
                }
            } else if (m_scanline_keeper.scanlines()[i].filter_type() == 3) { // Average
                int width = m_ihdr_chunk.width * m_scanline_keeper.color_length();
                for (int j = 0; j < width; j++) {
                    int prev = 0;
                    int prior = 0;
                    if (i > 0) {
                        prior = m_scanline_keeper.scanlines()[i - 1].data()[j];
                    }
                    if (j >= m_scanline_keeper.color_length()) {
                        prev = m_scanline_keeper.scanlines()[i].data()[j - m_scanline_keeper.color_length()];
                    }
                    m_scanline_keeper.scanlines()[i].data()[j] += (prev + prior) / 2;
                }
            } else if (m_scanline_keeper.scanlines()[i].filter_type() == 4) { // Paeth
                int width = m_ihdr_chunk.width * m_scanline_keeper.color_length();
                for (int j = 0; j < width; j++) {
                    int a = 0;
                    int b = 0;
                    int c = 0;
                    if (i > 0) {
                        b = m_scanline_keeper.scanlines()[i - 1].data()[j];
                    }
                    if (j >= m_scanline_keeper.color_length()) {
                        a = m_scanline_keeper.scanlines()[i].data()[j - m_scanline_keeper.color_length()];
                        if (i > 0) {
                            c = m_scanline_keeper.scanlines()[i - 1].data()[j - m_scanline_keeper.color_length()];
                        }
                    }
                    m_scanline_keeper.scanlines()[i].data()[j] += paeth_predictor(a, b, c);
                }
            }
        }
    }

    void PNGLoader::copy_scanlines_to_bitmap(PixelBitmap& bitmap)
    {
        if (m_ihdr_chunk.color_type == 2) {
            bitmap.set_format(PixelBitmapFormat::RGB);
            for (int i = 0; i < m_ihdr_chunk.height; i++) {
                auto& scanline = m_scanline_keeper.scanlines()[i];
                for (int j = 0, bit = 0; j < m_ihdr_chunk.width; j++) {
                    int r = scanline.data()[bit++];
                    int g = scanline.data()[bit++];
                    int b = scanline.data()[bit++];
                    int alpha = 255;
                    bitmap[i][j] = Color(r, g, b, alpha);
                }
            }
        }
        if (m_ihdr_chunk.color_type == 6) {
            bitmap.set_format(PixelBitmapFormat::RGBA);
            for (int i = 0; i < m_ihdr_chunk.height; i++) {
                auto& scanline = m_scanline_keeper.scanlines()[i];
                for (int j = 0, bit = 0; j < m_ihdr_chunk.width; j++) {
                    int r = scanline.data()[bit++];
                    int g = scanline.data()[bit++];
                    int b = scanline.data()[bit++];
                    int alpha = scanline.data()[bit++];
                    bitmap[i][j] = Color(r, g, b, alpha);
                }
            }
        }
    }

    bool PNGLoader::read_chunk(PixelBitmap& bitmap)
    {
        ChunkHeader header;
        streamer().read(header.len);
        streamer().read(header.type, 4);

        if (memcmp(header.type, (uint8_t*)"IHDR", 4) == 0) {
            read_IHDR(header, bitmap);
        } else if (memcmp(header.type, (uint8_t*)"tEXt", 4) == 0) {
            read_TEXT(header, bitmap);
        } else if (memcmp(header.type, (uint8_t*)"zTXt", 4) == 0) {
            read_TEXT(header, bitmap);
        } else if (memcmp(header.type, (uint8_t*)"pHYs", 4) == 0) {
            read_PHYS(header, bitmap);
        } else if (memcmp(header.type, (uint8_t*)"sRGB", 4) == 0) {
            read_PHYS(header, bitmap);
        } else if (memcmp(header.type, (uint8_t*)"eXIf", 4) == 0) {
            read_PHYS(header, bitmap);
        } else if (memcmp(header.type, (uint8_t*)"orNT", 4) == 0) {
            read_ORNT(header, bitmap);
        } else if (memcmp(header.type, (uint8_t*)"IDAT", 4) == 0) {
            read_IDAT(header, bitmap);
        } else if (memcmp(header.type, (uint8_t*)"gAMA", 4) == 0) {
            read_gAMA(header, bitmap);
        } else if (memcmp(header.type, (uint8_t*)"IEND", 4) == 0) {
            return false;
        } else {
            Logger::debug << "PNGLoader: Unexpected header type: " << (char*)header.type << std::endl;
            return false;
        }

        int crc;
        streamer().read(crc);

        return true;
    }

    void PNGLoader::proccess_stream(PixelBitmap& bitmap)
    {
        int len = 0;
        while (read_chunk(bitmap)) { }
        process_compressed_data(bitmap);
    }

    PixelBitmap PNGLoader::load_from_mem(const uint8_t* ptr)
    {
        auto bitmap = PixelBitmap();
        if (!ptr) {
            Logger::debug << "PNGLoader: nullptr" << std::endl;
            return bitmap;
        }

        if (!check_header(ptr)) {
            Logger::debug << "PNGLoader: not png" << std::endl;
            return bitmap;
        }

        streamer().set(ptr + png_header_size);
        proccess_stream(bitmap);
        return bitmap;
    }

} // namespace PNG
} // namespace LG