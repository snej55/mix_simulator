#ifndef APP_H
#define APP_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>

#include "engine.h"
#include "window.h"
#include "renderer.h"

class App : public EngineObject
{
public:
    App();
    ~App() override;

    virtual bool init(int width, int height, const char* title);
    
    void free() override;
    
    virtual void update();
    
    // --- window functions ---
    virtual bool createWindow(int width, int height, const char* title); // initializes window
    [[nodiscard]] Window* getWindow() const {return m_window;}

    virtual bool createRenderer();
    [[nodiscard]] Renderer* getRenderer() const {return m_renderer;}

    // get time info from Window::Clock
    [[nodiscard]] float getDeltaTime() const;
    [[nodiscard]] float getTime() const;

private:
    Window* m_window{nullptr};
    Renderer* m_renderer{nullptr};
};

#endif