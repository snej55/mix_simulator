#ifndef WINDOW_H
#define WINDOW_H

#include <GLFW/glfw3.h>

#include <string>

#include "engine_types.hpp"

class Window final : public EngineObject
{
public:
    explicit Window(EngineObject* parent); // initialize EngineObject
    ~Window() override; // free

    // initializes glfw window
    bool init(int width, int height, const char* title);

    // free resources
    void free();

    // create viewport and setup glfw callbacks
    void createViewPort();

    // --- Window management stuff ---
    // tell window whether to quit or not
    void setQuit(const bool val) { m_quit = val; };
    // check whether window has been told to quit
    [[nodiscard]] bool getQuit() const { return m_quit; }
    // checks if window should close
    [[nodiscard]] bool getShouldClose() const;
    // clear screen
    void clear() const;
    // swap buffers
    void tick() const;

    // getters & setters
    [[nodiscard]] int getWidth() const { return m_width; }
    [[nodiscard]] int getHeight() const { return m_height; }
    [[nodiscard]] std::string_view getTitle() const { return m_title; };

    // width & height setters
    void setWidth(const int& val) { m_width = val; }
    void setWidth(const int&& val) { m_width = val; }
    void setHeight(const int& val) { m_height = val; }
    void setHeight(const int&& val) { m_height = val; }

    void updateDimensions();

    // title setters
    void setTitle(const char* title);
    void setTitle(std::string_view title);
    void setTitle(const std::string& title);

    // returns pointer to GLFWWindow member
    [[nodiscard]] GLFWwindow* getWindow() const { return m_window; }

    // window callbacks
    void framebuffer_size_callback(int width, int height);
    void mouse_callback(double xpos, double ypos) const;
    void scroll_callback(double yoffset) const;

private:
    int m_width{0};
    int m_height{0};
    std::string m_title{};

    // flags
    bool m_quit{false}; // quit

    GLFWwindow* m_window{nullptr};

    // glfw callbacks
    static void win_framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void win_mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void win_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};

#endif
