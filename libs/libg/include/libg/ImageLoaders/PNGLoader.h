/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <libfoundation/ByteOrder.h>
#include <libg/Color.h>
#include <libg/PixelBitmap.h>
#include <libg/Rect.h>
#include <string>

namespace LG {
namespace PNG {

    struct ChunkHeader {
        uint32_t len;
        uint8_t type[4];
    };

    struct IHDRChunk {
        uint32_t width;
        uint32_t height;
        uint8_t depth;
        uint8_t color_type;
        uint8_t compression_method;
        uint8_t filter_method;
        uint8_t interlace_method;
    };

    class DataStreamer {
    public:
        DataStreamer() = default;

        DataStreamer(const void* data)
            : m_ptr((uint8_t*)data)
        {
        }

        ~DataStreamer() = default;

        template <typename T>
        void read(T& val)
        {
            val = *(T*)m_ptr;
            m_ptr += sizeof(T);
            val = LFoundation::ByteOrder::from_network(val);
        }

        void read(void* buffer, size_t cnt)
        {
            memcpy((uint8_t*)buffer, m_ptr, cnt);
            m_ptr += cnt;
        }

        template <typename T>
        T at(int index) const
        {
            T val = *((T*)m_orig_ptr + index);
            val = LFoundation::ByteOrder::from_network(val);
            return val;
        }

        void skip(size_t cnt) { m_ptr += cnt; }
        void set(const void* data) { m_orig_ptr = m_ptr = (uint8_t*)data; }
        const uint8_t* ptr() const { return m_ptr; }
        uint8_t* ptr() { return m_ptr; }

    private:
        uint8_t* m_orig_ptr { nullptr };
        uint8_t* m_ptr { nullptr };
    };

    class Scanline {
    public:
        Scanline() = default;
        Scanline(int type, void* ptr)
            : m_filter_type(type)
            , m_ptr(ptr)
        {
        }
        ~Scanline() = default;

        void set(int type, void* ptr) { m_ptr = ptr; }
        int filter_type() const { return m_filter_type; }
        uint8_t* data() const { return (uint8_t*)m_ptr; }

    private:
        int m_filter_type { 0 };
        void* m_ptr { nullptr };
        size_t m_len { 0 };
    };

    class ScanlineKeeper {
    public:
        ScanlineKeeper() = default;
        ~ScanlineKeeper() { invalidate(); }

        inline void init(void* ptr) { m_ptr = ptr; }
        inline void init(void* ptr, uint8_t color_length) { m_ptr = ptr, m_color_length = color_length; }

        void invalidate()
        {
            if (m_ptr) {
                free(m_ptr);
                m_data.clear();
                m_ptr = nullptr;
            }
        }

        inline void add(Scanline&& el) { m_data.push_back(std::move(el)); }
        inline void set_color_length(uint8_t color_length) { m_color_length = color_length; }
        inline uint8_t color_length() const { return m_color_length; }

        inline std::vector<Scanline>& scanlines() { return m_data; }
        inline const std::vector<Scanline>& scanlines() const { return m_data; }

    private:
        uint8_t m_color_length { 0 };
        void* m_ptr { nullptr };
        std::vector<Scanline> m_data;
    };

    class PNGLoader {
    public:
        PNGLoader() = default;
        ~PNGLoader() = default;

        PixelBitmap load_from_file(const std::string& path);
        PixelBitmap load_from_mem(const uint8_t* ptr);

        inline DataStreamer& streamer() { return m_streamer; }

    private:
        bool check_header(const uint8_t* ptr) const;

        void proccess_stream(PixelBitmap& bitmap);
        void process_compressed_data(PixelBitmap& bitmap);
        bool read_chunk(PixelBitmap& bitmap);
        void read_IHDR(ChunkHeader& header, PixelBitmap& bitmap);
        void read_TEXT(ChunkHeader& header, PixelBitmap& bitmap);
        void read_PHYS(ChunkHeader& header, PixelBitmap& bitmap);
        void read_ORNT(ChunkHeader& header, PixelBitmap& bitmap);
        void read_IDAT(ChunkHeader& header, PixelBitmap& bitmap);

        uint8_t paeth_predictor(int a, int b, int c);
        void unfilter_scanlines();
        void copy_scanlines_to_bitmap(PixelBitmap& bitmap);

        std::vector<uint8_t> m_compressed_data;
        DataStreamer m_streamer;
        IHDRChunk m_ihdr_chunk;
        ScanlineKeeper m_scanline_keeper;
    };

} // namespace PNG
} // namespace LG