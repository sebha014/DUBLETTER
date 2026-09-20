#pragma once
#include "window.h"
#include "os.h"

#ifndef WINDOWS

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

/**
 * Version of the Window class that uses SFML.
 */
class SFML_Window : public Window {
public:
    // Create and destroy.
    SFML_Window();
    virtual ~SFML_Window();

    // Show an image.
    void show_multiple(const std::vector<std::string> &captions,
                    const std::vector<Image> &images,
                    bool pause = true) override;

private:
    // Thread that manages the window.
    std::thread window_thread;


    /**
     * State managed by the window thread.
     */

    // The SFML window we are using.
    std::unique_ptr<sf::RenderWindow> window;

    // Font used for creating text.
    sf::Font font;

    // Current set of images we are showing.
    std::vector<std::unique_ptr<sf::Texture>> images;

    // Captions for the images.
    std::vector<std::unique_ptr<sf::Text>> captions;

    // Main function for the window thread.
    void window_main();

    // Process any message in the 'message' variable.
    void process_message();

    // Create a sf::Texture from an Image.
    std::unique_ptr<sf::Texture> create_texture(const Image &image);

    // Draw to the window.
    void draw();
    void draw_image(sf::Rect<float> pos, size_t index);


    /**
     * State shared between the threads:
     */

    // Lock and condition variable for shared state.
    std::mutex state_lock;
    std::condition_variable state_cond;

    // Is the thread running and ready to receive messages?
    bool thread_ready;

    // Should waiting thread(s) stop waiting?
    bool stop_waiting;

    // Is the window closed?
    bool window_closed;

    // Request the window thread to exit.
    bool request_exit;

    // Message to the window thread, polled during vsync.
    class Message;
    Message *message;
};

#endif
