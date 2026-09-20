#include "sfml_window.h"
#include <iostream>
#include <cmath>

#ifndef WINDOWS

class SFML_Window::Message {
public:
    const std::vector<std::string> *captions;
    const std::vector<Image> *images;
};

SFML_Window::SFML_Window() : thread_ready{false}, stop_waiting{true}, message{nullptr} {
    // Note: Not in the initializer list to ensure that all members are initialized by the time the
    // thread is created.
    window_thread = std::thread(&SFML_Window::window_main, this);

    std::unique_lock<std::mutex> u{state_lock};
    while (!thread_ready)
        state_cond.wait(u);
}

SFML_Window::~SFML_Window() {
    {
        std::unique_lock<std::mutex> u{state_lock};
        request_exit = true;
    }

    window_thread.join();
}

void SFML_Window::window_main() {
#if SFML_VERSION_MAJOR >= 3
    sf::VideoMode video_mode({1000, 700});
#else
    sf::VideoMode video_mode{1000, 700};
#endif
    window = std::make_unique<sf::RenderWindow>(video_mode, "Images");
    window->setVerticalSyncEnabled(true);

    // Some font options with fallbacks.
    std::vector<const char *> fonts = {
        "Ubuntu.ttf",
        "../Ubuntu.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        nullptr,
    };

    for (size_t i = 0; i < fonts.size(); i++) {
        if (!fonts[i]) {
            std::cerr << "Could not load a font. Text will not be visible!" << std::endl;
            break;
        }

#if SFML_VERSION_MAJOR >= 3
        if (font.openFromFile(fonts[i])) {
            break;
        }
#else
        if (font.loadFromFile(fonts[i])) {
            break;
        }
#endif
    }


    {
        std::unique_lock<std::mutex> u{state_lock};
        thread_ready = true;
        state_cond.notify_all();
    }

    bool quit = false;
    while (!quit) {
#if SFML_VERSION_MAJOR >= 3
        while (const std::optional event = window->pollEvent()) {
            if (const auto *resize = event->getIf<sf::Event::Resized>()) {
                window->setView(sf::View(sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(resize->size))));
            } else if (event->is<sf::Event::Closed>()) {
                std::unique_lock<std::mutex> u{state_lock};
                stop_waiting = true;
                quit = true;
                state_cond.notify_all();
                break;
            } else if (event->is<sf::Event::MouseButtonReleased>()) {
                std::unique_lock<std::mutex> u{state_lock};
                stop_waiting = true;
                state_cond.notify_all();
            }
        }
#else
        sf::Event event;
        while (window->pollEvent(event)) {
            switch (event.type) {
            case sf::Event::Resized:
                window->setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
                break;
            case sf::Event::Closed:
            {
                std::unique_lock<std::mutex> u{state_lock};
                stop_waiting = true;
                quit = true;
                state_cond.notify_all();
                break;
            }
            case sf::Event::MouseButtonReleased:
            {
                std::unique_lock<std::mutex> u{state_lock};
                stop_waiting = true;
                state_cond.notify_all();
                break;
            }
            default:
                break;
            }
        }
#endif

        {
            std::unique_lock<std::mutex> u{state_lock};
            if (request_exit)
                quit = true;
        }

        process_message();

        window->clear();
        draw();
        window->display();
    }

    {
        std::unique_lock<std::mutex> u{state_lock};
        window_closed = true;
        state_cond.notify_all();
    }

    // Destroy the window in the thread that created it.
    window = std::unique_ptr<sf::RenderWindow>();
}

void SFML_Window::process_message() {
    std::unique_lock<std::mutex> u{state_lock};
    if (!message)
        return;

    images.clear();
    images.reserve(message->images->size());
    captions.clear();
    captions.reserve(message->images->size());

    for (size_t i = 0; i < message->images->size(); i++) {
        images.push_back(create_texture(message->images->at(i)));

        std::string src;
        if (i < message->captions->size())
            src = message->captions->at(i);

#if SFML_VERSION_MAJOR >= 3
        captions.push_back(std::make_unique<sf::Text>(font, src, 18));
#else
        captions.push_back(std::make_unique<sf::Text>(src, font, 18));
#endif
    }

    message = nullptr;
    state_cond.notify_all();
}

static int to_byte(float f) {
    int b = static_cast<int>(f * 255.0f);
    if (b < 0)
        b = 0;
    if (b > 255)
        b = 255;
    return b;
}

std::unique_ptr<sf::Texture> SFML_Window::create_texture(const Image &image) {
    std::unique_ptr<sf::Texture> result = std::make_unique<sf::Texture>();
#if SFML_VERSION_MAJOR >= 3
    (void)result->resize(sf::Vector2u(image.width(), image.height()));
#else
    result->create(image.width(), image.height());
#endif

    uint8_t *pixels = new uint8_t[image.width() * image.height() * 4];
    size_t offset = 0;
    for (size_t y = 0; y < image.height(); y++) {
        for (size_t x = 0; x < image.width(); x++) {
            Pixel px = image.pixel(x, y);
            pixels[offset++] = to_byte(px.r);
            pixels[offset++] = to_byte(px.g);
            pixels[offset++] = to_byte(px.b);
            pixels[offset++] = 255; // alpha
        }
    }

    result->update(pixels);
    delete []pixels;

    result->setSmooth(true);
    return result;
}

void SFML_Window::draw() {
    const size_t x_margin = 8;
    const size_t y_margin = 0;

    sf::Vector2u size = window->getSize();
    size.x -= x_margin * 2;
    size.y -= y_margin * 2;

    size_t grid_width = static_cast<size_t>(std::ceil(std::sqrt(static_cast<double>(images.size()))));
    if (grid_width <= 0)
        grid_width = 1;
    size_t grid_height = (images.size() + grid_width - 1) / grid_width;

    size_t i = 0;
    for (size_t y = 0; y < grid_height; y++) {
        size_t line_width = grid_width;
        if (images.size() - i < line_width)
            line_width = images.size() - i;

        for (size_t x = 0; x < line_width; x++) {
#if SFML_VERSION_MAJOR >= 3
            sf::Rect<float> pos{
                {
                    x_margin + static_cast<float>(size.x * x) / line_width,
                    y_margin + static_cast<float>(size.y * y) / grid_height,
                }, {
                    static_cast<float>(size.x) / line_width,
                    static_cast<float>(size.y) / grid_height
                }
            };
#else
            sf::Rect<float> pos{
                x_margin + static_cast<float>(size.x * x) / line_width,
                y_margin + static_cast<float>(size.y * y) / grid_height,
                static_cast<float>(size.x) / line_width,
                static_cast<float>(size.y) / grid_height
            };
#endif

            draw_image(pos, i);
            i++;
        }
    }
}

#if SFML_VERSION_MAJOR >= 3
void SFML_Window::draw_image(sf::Rect<float> pos, size_t index) {
    pos.position.y += 8;
    pos.size.y -= 8;

    sf::Text &text = *captions[index];
    sf::Rect<float> textBounds = text.getGlobalBounds();
    text.setOrigin({textBounds.size.x / 2, 0});
    text.setPosition({pos.position.x + pos.size.x / 2, pos.position.y});
    text.setFillColor(sf::Color::White);
    window->draw(text);

    pos.position.y += textBounds.size.y + 8;
    pos.size.y -= textBounds.size.y + 8;

    sf::Texture &image = *images[index];

    sf::Vector2u image_size = image.getSize();
    float scale = std::min(pos.size.x / image_size.x, pos.size.y / image_size.y);
    if (scale > 1.0f)
        scale = 1.0f;

    sf::RectangleShape shape(sf::Vector2f(image_size) * scale);
    shape.setOrigin({std::ceil(shape.getSize().x / 2), std::ceil(shape.getSize().y / 2)});
    shape.setPosition({std::floor(pos.position.x + pos.size.x / 2), std::floor(pos.position.y + pos.size.y / 2)});
    shape.setTexture(&image);
    window->draw(shape);
}
#else
void SFML_Window::draw_image(sf::Rect<float> pos, size_t index) {
    pos.top += 8;
    pos.height -= 8;

    sf::Text &text = *captions[index];
    sf::Rect<float> textBounds = text.getGlobalBounds();
    text.setOrigin(textBounds.width / 2, 0);
    text.setPosition(pos.left + pos.width / 2, pos.top);
    text.setFillColor(sf::Color::White);
    window->draw(text);

    pos.top += textBounds.height + 8;
    pos.height -= textBounds.height + 8;

    sf::Texture &image = *images[index];

    sf::Vector2u image_size = image.getSize();
    float scale = std::min(pos.width / image_size.x, pos.height / image_size.y);
    if (scale > 1.0f)
        scale = 1.0f;

    sf::RectangleShape shape(sf::Vector2f(image_size) * scale);
    shape.setOrigin(std::ceil(shape.getSize().x / 2), std::ceil(shape.getSize().y / 2));
    shape.setPosition(std::floor(pos.left + pos.width / 2), std::floor(pos.top + pos.height / 2));
    shape.setTexture(&image);
    window->draw(shape);
}
#endif

void SFML_Window::show_multiple(const std::vector<std::string> &captions,
                            const std::vector<Image> &images,
                            bool pause) {
    if (disable_pause)
        pause = false;

    Message msg = { &captions, &images };
    {
        std::unique_lock<std::mutex> u{state_lock};
        if (!window_closed) {
            while (message)
                state_cond.wait(u);

            message = &msg;
            stop_waiting = false;

            while (message)
                state_cond.wait(u);
        }
    }

    if (pause) {
        std::unique_lock<std::mutex> u{state_lock};
        while (!stop_waiting && !window_closed)
            state_cond.wait(u);
    }
}

#endif
