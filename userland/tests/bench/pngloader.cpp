#include "common.h"
#include <libg/ImageLoaders/PNGLoader.h>

LG::PixelBitmap bitmap;

void bench_pngloader()
{
    RUN_BENCH("PNG LOADER", 5)
    {
        LG::PNG::PNGLoader loader;
        bitmap = loader.load_from_file("/res/wallpapers/island.png");
    }
}