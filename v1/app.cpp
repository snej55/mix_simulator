#include "app.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <cassert>

App::App()
 : EngineObject{nullptr, "App"}
{
}

App::~App()
{
    if (!m_freed)
    {
        App::free();
    }
}

bool App::init(const int width, const int height, const char* title)
{
    // initialize opengl context
    glfwInit();
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // initialize window
    if (!createWindow(width, height, title))
    {
        std::cout << "APP::INIT::ERROR: Unable to initialize new window!" << std::endl;
        glfwTerminate();
        return false;
    }

    std::cout << "Successfully initialized GLFW!\n";

    // initialize glad
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD!" << std::endl;
        return false;
    }

    std::cout << "Successfully initialized GLAD!\n";

    // --- INITIALIZE WINDOW OBJECTS --- //

    // create view port and add window callbacks
    m_window->createViewPort();
    // allow keyboard input and stuff
    if (!m_window->createIOHandler())
    {
        std::cout << "Failed to create IOHandler!" << std::endl;
        return false;
    }
    // create clock for deltatime, etc
    if (!m_window->createClock())
    {
        std::cout << "Failed to create Clock!" << std::endl;
        return false;
    }

    // --- INITIALIZE RENDERER --- //
    if (!createRenderer())
    {
        std::cout << "Failed to create renderer!" << std::endl;
        return false;
    }

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    return true;
}

void App::free()
{
    EngineObject::free();
    glfwTerminate();
    std::cout << "Terminated OpenGL context!" << std::endl;
}

void App::update()
{
    assert(m_window != nullptr);
    m_window->update();
}

bool App::createWindow(const int width, const int height, const char* title)
{
    if (m_window == nullptr)
    {
        m_window = new Window{this};
        return m_window->init(width, height, title);
    }
    std::cout << "APP::CREATE_WINDOW::ERROR: Window already exists!" << std::endl;
    return false;
}

bool App::createRenderer()
{
    if (m_renderer == nullptr)
    {
        m_renderer = new Renderer{this};
        return m_renderer->init();
    }
    std::cout << "APP::CREATE_RENDERER::ERROR: Renderer already exists!" << std::endl;
    return false;
}

// --- WINDOW FUNCTIONS --- //

// get delta time from window clock
float App::getDeltaTime() const
{
    return m_window->getDeltaTime();
}

float App::getTime() const
{
    return m_window->getTime();
}
