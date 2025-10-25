#include "window.h"

#include <iostream>
#include <cassert>

// initialize EngineObject
Window::Window(EngineObject* parent)
 : EngineObject{parent, "Window"}
{
}

// free
Window::~Window()
{
    if (!m_freed)
    {
        Window::free();
    }
}

// initialize glfw window
bool Window::init(const int width, const int height, const char* title)
{
    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    // validate window
    if (m_window == nullptr)
    {
        std::cout << "Failed to create GLFW window!" << std::endl;
        return false;
    }
    glfwMakeContextCurrent(m_window);

    setWidth(width);
    setHeight(height);
    setTitle(title);

    std::cout << "Created GLFW window: {dimensions: " << width << " * " << height << ", title: '" << title << "'}\n";

    return true;
}

// free resources
void Window::free()
{
    EngineObject::free();
    glfwDestroyWindow(m_window);
    m_window = nullptr;
    std::cout << "Destroyed GLFW window!\n";
}

// updates objects
void Window::update()
{
    assert(m_window != nullptr);

    // update iohandler
    if (m_iohandler != nullptr)
    {
        m_iohandler->update();
    }
}

// creates the viewport and setups glfw callbacks
void Window::createViewPort()
{
    glViewport(0, 0, m_width, m_height);

    glfwSetWindowUserPointer(m_window, this);

    glfwSetFramebufferSizeCallback(m_window, win_framebuffer_size_callback);
    glfwSetCursorPosCallback(m_window, win_mouse_callback);
    glfwSetScrollCallback(m_window, win_scroll_callback);
}

// creates new IOHandler
bool Window::createIOHandler()
{
    if (m_iohandler == nullptr)
    {
        m_iohandler = new IOHandler{this};
        return true;
    }
    std::cout << "APP::WINDOW::CREATE_IOHANDLER::ERROR: IOHandler already exists!" << std::endl;
    return false;
}

bool Window::createClock()
{
    if (m_clock == nullptr)
    {
        m_clock = new Clock{this};
        return true;
    }
    std::cout << "APP::WINDOW::CREATE_CLOCK::ERROR: Clock already exists!" << std::endl;
    return false;
}

// checks if user has pressed esc or something
bool Window::getQuit() const
{
    if (m_iohandler != nullptr)
    {
        return m_iohandler->getQuit();
    }
    return false;
}

// checks if window should close
bool Window::getShouldClose() const
{
    return glfwWindowShouldClose(m_window) || getQuit();
}

// clear screen
void Window::clear() const
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Window::tick()
{
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

// Get key state for GLFW key from IOHandler
bool Window::getPressed(const int key) const
{
    assert(m_iohandler != nullptr);
    return m_iohandler->getPressed(key);
}

float Window::getDeltaTime() const
{
    assert(m_clock != nullptr);
    return m_clock->getDeltaTime();
}

float Window::getTime() const
{
    assert(m_clock != nullptr);
    return m_clock->getTime();
}

void Window::setTitle(const char* title)
{
    m_title = title;
    glfwSetWindowTitle(m_window, title);
}

void Window::setTitle(const std::string_view title)
{
    m_title = title;
    // convert to c-string for glfw
    const char* titleChr = m_title.c_str();
    glfwSetWindowTitle(m_window, titleChr);
}

void Window::setTitle(const std::string& title)
{
    m_title = title;
    // convert to c-string for glfw
    const char* titleChr = m_title.c_str();
    glfwSetWindowTitle(m_window, titleChr);
}

// CALLBACKS
// window callbacks
void Window::framebuffer_size_callback(const int width, const int height)
{
    setWidth(width);
    setHeight(height);
    glViewport(0, 0, width, height);
}

// gets called from glfw cursor pos callback
void Window::mouse_callback(const double xpos, const double ypos)
{
    // pass
    return;
}

// gets called from glfw scroll callback
void Window::scroll_callback(const double xoffset, const double yoffset)
{
    // pass
    return;
}

// glfw callbacks
void Window::win_framebuffer_size_callback(GLFWwindow* window, const int width, const int height)
{
    if (Window* handler{static_cast<Window*>(glfwGetWindowUserPointer(window))})
    {
        handler->framebuffer_size_callback(width, height);
    }
}

void Window::win_mouse_callback(GLFWwindow* window, const double xpos, const double ypos)
{
    if (Window* handler{static_cast<Window*>(glfwGetWindowUserPointer(window))})
    {
        handler->mouse_callback(xpos, ypos);
    }
}

void Window::win_scroll_callback(GLFWwindow* window, const double xoffset, const double yoffset)
{
    if (Window* handler{static_cast<Window*>(glfwGetWindowUserPointer(window))})
    {
        handler->scroll_callback(xoffset, yoffset);
    }
}
