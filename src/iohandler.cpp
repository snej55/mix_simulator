#include "iohandler.hpp"

IOHandler::IOHandler(EngineObject* engine, GLFWwindow* window) : EngineObject{"IOHandler", engine}, m_window{window} {}

void IOHandler::update()
{
    if (getPressed(GLFW_KEY_ESCAPE))
    {
        m_quit = true;
    }
}

bool IOHandler::getPressed(const int key) const
{
    if (glfwGetKey(m_window, key) == GLFW_PRESS)
    {
        return true;
    }
    return false;
}
