#include "engine.hpp"
#include "window.hpp"
#include "util.hpp"

#include <iostream>

// initialize EngineObject
Window::Window(EngineObject* parent) : EngineObject{"Window", parent} {}

// free
Window::~Window() { free(); }

// create glfw window
bool Window::init(const int width, const int height, const char* title)
{
    // create glfw window
    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    // validate window
    if (m_window == nullptr)
    {
        Util::beginError();
        std::cout << "WINDOW::INIT::ERROR: Failed to create GLFW window!";
        Util::endError();
        return false;
    }

    // use this window
    glfwMakeContextCurrent(m_window);

    // setup rest of window
    setWidth(width);
    setHeight(height);
    setTitle(title); // implicit conversion

    std::cout << "WINDOW::INIT: Created GLFW window: {dimensions: " << width << " * " << height << ", title: " << title
              << "}\n";

    // success!
    return true;
}

void Window::free()
{
    glfwDestroyWindow(m_window);
    m_window = nullptr;
    std::cout << "WINDOW::FREE: Destroyed GLFW window!\n";
}

// create view port and setup glfw callbacks
void Window::createViewPort()
{
    glViewport(0, 0, static_cast<int>(m_width), static_cast<int>(m_height));

    glfwSetWindowUserPointer(m_window, this);

    glfwSetFramebufferSizeCallback(m_window, win_framebuffer_size_callback);
    glfwSetCursorPosCallback(m_window, win_mouse_callback);
    glfwSetScrollCallback(m_window, win_scroll_callback);

    std::cout << "WINDOW::CREATE_VIEW_PORT: Set GL viewport: " << m_width << " * " << m_height << '\n';
}

// checks if window should close
bool Window::getShouldClose() const { return glfwWindowShouldClose(m_window) || getQuit(); }

// clear screen
void Window::clear() const
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

// swap buffers
void Window::tick() const
{
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

// set glfw window title from c-str
void Window::setTitle(const char* title)
{
    m_title = title;
    glfwSetWindowTitle(m_window, title);
}

// set glfw window title from std::string_view
void Window::setTitle(const std::string_view title)
{
    m_title = title;
    // convert to c-string for glfw
    const char* titleChr = m_title.c_str();
    glfwSetWindowTitle(m_window, titleChr);
}

// set glfw window title from std::string
void Window::setTitle(const std::string& title)
{
    m_title = title;
    // convert to c-string for glfw
    const char* titleChr = m_title.c_str();
    glfwSetWindowTitle(m_window, titleChr);
}

// update framebuffer dimensions
void Window::updateDimensions()
{
    glfwGetFramebufferSize(m_window, &m_width, &m_height);
    glViewport(0, 0, m_width, m_height);
}

// -------- CALLBACKS -------- //
// window callbacks
void Window::framebuffer_size_callback(const int width, const int height)
{
    setWidth(width);
    setHeight(height);
    glViewport(0, 0, width, height);

    // update post processor
    dynamic_cast<Engine*>(m_parent)->updatePostProcessor(width, height);
}

// gets called from glfw cursor pos callback
void Window::mouse_callback(const double xpos, const double ypos) const
{
    dynamic_cast<Engine*>(m_parent)->mouse_callback(xpos, ypos);
}

void Window::scroll_callback(const double yoffset) const { dynamic_cast<Engine*>(m_parent)->scroll_callback(yoffset); }

// glfw callbacks
void Window::win_framebuffer_size_callback(GLFWwindow* window, const int width, const int height)
{
    if (Window * handler{static_cast<Window*>(glfwGetWindowUserPointer(window))})
    {
        handler->framebuffer_size_callback(width, height);
    }
}

void Window::win_mouse_callback(GLFWwindow* window, const double xpos, const double ypos)
{
    if (Window * handler{static_cast<Window*>(glfwGetWindowUserPointer(window))})
    {
        handler->mouse_callback(xpos, ypos);
    }
}

void Window::win_scroll_callback(GLFWwindow* window, const double xoffset, const double yoffset)
{
    if (Window * handler{static_cast<Window*>(glfwGetWindowUserPointer(window))})
    {
        handler->scroll_callback(yoffset);
    }
}
