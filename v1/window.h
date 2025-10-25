#ifndef WINDOW_H
#define WINDOW_H

#include "engine.h"
#include "iohandler.h"
#include "clock.h"

#include <GLFW/glfw3.h>

#include <string>

class Window : public EngineObject
{
public:
    explicit Window(EngineObject* parent); // initialize EngineObject
    ~Window() override; // free

    // initializes glfw window
    virtual bool init(int width, int height, const char* title);
    
    // free resources
    void free() override;
    
    // update objects
    virtual void update();

    // create viewport and setup glfw callbacks
    virtual void createViewPort();
    // create new IOHandler object
    virtual bool createIOHandler();
    virtual bool createClock();

    // --- Window management stuff ---
    // checks if window should close
    [[nodiscard]] bool getQuit() const;
    [[nodiscard]] bool getShouldClose() const;
    // clear screen
    virtual void clear() const;
    // swap buffers and calculate delta time
    virtual void tick();

    // io handler stuff
    [[nodiscard]] IOHandler* getIOHandler() const {return m_iohandler;}
    // get state of keyboard keys from iohandler
    [[nodiscard]] bool getPressed(int key) const;

    // clock stuff
    [[nodiscard]] Clock* getClock() const {return m_clock;}
    [[nodiscard]] float getDeltaTime() const;
    [[nodiscard]] float getTime() const;

    // getters & setters
    [[nodiscard]] int getWidth() const {return m_width;}
    [[nodiscard]] int getHeight() const {return m_height;}
    [[nodiscard]] std::string_view getTitle() const {return m_title;};

    // width & height setters
    void setWidth(const int& val) {m_width = val;}
    void setWidth(const int&& val) {m_width = val;}
    void setHeight(const int& val) {m_height = val;}
    void setHeight(const int&& val) {m_height = val;}

    // title setters
    void setTitle(const char* title);
    void setTitle(const std::string_view title);
    void setTitle(const std::string& title);

    // returns pointer to GLFWWindow member
    [[nodiscard]] GLFWwindow* getWindow() const {return m_window;}

    // window callbacks
    void framebuffer_size_callback(int width, int height);
    void mouse_callback(double xpos, double ypos);
    void scroll_callback(double xoffset, double yoffset);

private:
    int m_width{0};
    int m_height{0};

    std::string m_title{};

    GLFWwindow* m_window{nullptr};

    IOHandler* m_iohandler{nullptr};
    Clock* m_clock{nullptr};

    // glfw callbacks
    static void win_framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void win_mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void win_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};

#endif
