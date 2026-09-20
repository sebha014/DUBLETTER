#include "load.h"
#include "os.h"
#include <algorithm>
#include <filesystem>

std::vector<std::string> list_files(const std::string &dir, const std::vector<std::string> &types) {
    std::vector<std::string> result;

    for (auto const &entry : std::filesystem::directory_iterator{dir}) {
        if (entry.is_directory())
            continue;

        std::string ext = entry.path().extension().string();
        if (types.empty() || std::find(types.begin(), types.end(), ext) != types.end())
            result.push_back(entry.path().string());
    }

    std::sort(result.begin(), result.end());
    return result;
}

#ifdef WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincodec.h>
#pragma comment (lib, "ole32.lib")
#pragma comment (lib, "windowscodecs.lib")

typedef unsigned char byte;

class CoInit {
public:
    CoInit() {
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    }
};

static void release(IUnknown *resource) {
    if (resource)
        resource->Release();
}

static Image copyImage(IWICBitmapSource *source) {
    unsigned int width, height;
    source->GetSize(&width, &height);

    Image result(width, height);
    std::vector<byte> buffer(width * height * 4, 0);

    source->CopyPixels(NULL, width * 4, width * height * 4, &buffer[0]);

    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            size_t index = (x + y*width) * 4;
            result.pixel(x, y) = Pixel::from_bytes(buffer[index+2], buffer[index+1], buffer[index]);
        }
    }

    return result;
}

Image load_image(const std::string &file) {
    static CoInit coInit;

    const char *message = "Failed to initialize Windows Imaging Components.";
    IWICImagingFactory *wicFactory = NULL;
    HRESULT r = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));

    IWICBitmapDecoder *decoder = NULL;
    if (SUCCEEDED(r)) {
        std::wstring wide(file.size(), ' ');
        for (size_t i = 0; i < file.size(); i++)
            wide[i] = file[i];

        r = wicFactory->CreateDecoderFromFilename(wide.c_str(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
        message = "Failed to decode the input file.";
    }

    IWICBitmapFrameDecode *frame = NULL;
    if (SUCCEEDED(r)) {
        r = decoder->GetFrame(0, &frame);
        message = "Failed to get a frame from the file.";
    }

    IWICBitmapSource *bitmapSource = NULL;
    if (SUCCEEDED(r)) {
        r = frame->QueryInterface(IID_PPV_ARGS(&bitmapSource));
        message = "Failed to extract a bitmap source.";
    }

    IWICFormatConverter *fmtConverter = NULL;
    if (SUCCEEDED(r)) {
        r = wicFactory->CreateFormatConverter(&fmtConverter);
        message = "Failed to create a format converter.";
    }

    if (SUCCEEDED(r)) {
        r = fmtConverter->Initialize(bitmapSource, GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
        message = "Failed to convert the image.";
    }

    if (SUCCEEDED(r)) {
        release(bitmapSource);
        r = fmtConverter->QueryInterface(IID_PPV_ARGS(&bitmapSource));
        message = "Failed to get the new image.";
    }

    Image result(0, 0);
    if (SUCCEEDED(r)) {
        unsigned int width, height;
        bitmapSource->GetSize(&width, &height);

        result = copyImage(bitmapSource);
    }

    release(fmtConverter);
    release(bitmapSource);
    release(frame);
    release(decoder);
    release(wicFactory);

    if (!SUCCEEDED(r))
        throw Image_Load_Error(message);

    return result;
}

#else

#include <SFML/Graphics.hpp>

static Pixel from_sfml(sf::Color color) {
    return Pixel::from_bytes(color.r, color.g, color.b);
}

Image load_image(const std::string &file) {
    sf::Image image;
    if (!image.loadFromFile(file))
        throw Image_Load_Error("Failed to load the image. Is the path correct?");

    sf::Vector2u size = image.getSize();
    Image result(size.x, size.y);

    for (size_t y = 0; y < size.y; y++) {
        for (size_t x = 0; x < size.x; x++) {
#if SFML_VERSION_MAJOR >= 3
            sf::Color color = image.getPixel(sf::Vector2u(x, y));
#else
            sf::Color color = image.getPixel(x, y);
#endif
            result.pixel(x, y) = from_sfml(color);
        }
    }

    return result;
}

#endif
