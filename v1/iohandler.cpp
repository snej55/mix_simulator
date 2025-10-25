#include "iohandler.h"
#include "window.h"

IOHandler::IOHandler(EngineObject* window)
 : EngineObject{window, "IOHandler"}, m_window{window}
{
}

void IOHandler::update()
{
    if (getPressed(GLFW_KEY_ESCAPE))
    {
        m_quit = true;
    }
}

bool IOHandler::getPressed(const int key) const
{
    if (glfwGetKey(getParent<Window>()->getWindow(), key) == GLFW_PRESS)
    {
        return true;
    }
    return false;
}