//
// Created by Jens Kromdijk on 08/06/25.
//
// Handles IO input

#ifndef IOHANDLER_H
#define IOHANDLER_H

#include <GLFW/glfw3.h>

#include <type_traits>
#include <unordered_map>

#include "engine_types.hpp"
#include "util.hpp"

namespace IO
{
    struct ControlHash
    {
        template <typename Control>
        inline typename std::enable_if<std::is_enum<Control>::value, std::size_t>::type
        operator()(const Control value) const
        {
            return static_cast<std::size_t>(value);
        }
    };

    template <typename T, typename = void>
    struct contains_none : std::false_type
    {
    };

    template <typename T>
    struct contains_none<T, std::void_t<decltype(T::NONE)>> : std::true_type
    {
    };
} // namespace IO

template <typename Control>
class Controller
{
public:
    Controller()
    {
        if constexpr (IO::contains_none<Control>::value)
        {
            for (std::size_t i{0}; i < static_cast<std::size_t>(Control::NONE); ++i)
            {
                m_controls[static_cast<Control>(i)] = false;
            }
        }
        else
        {
            Util::beginError();
            std::cout << "IO::CONTROLLER::ERROR: Control::NONE must exist!";
            Util::endError();
        }
    }
    Controller(const Controller& other) { m_controls = other.getControlMap(); }
    Controller& operator=(const Controller& other) { m_controls = other.getControlMap(); }

    virtual ~Controller() = default;

    virtual void setControl(const Control control, const bool val)
    {
        if (m_controls.find(control) != m_controls.end())
        {
            m_controls[control] = val;
        }
        else
        {
            Util::beginError();
            std::cout << "IO::CONTROLLER::ERROR: Could not find control (" << static_cast<std::size_t>(control) << ")";
            Util::endError();
        }
    }

    virtual bool getControl(const Control control) const
    {
        auto it{m_controls.find(control)};
        if (it != m_controls.end())
        {
            return it->second;
        }

        Util::beginError();
        std::cout << "IO::CONTROLLER::ERROR: Could not find control (" << static_cast<std::size_t>(control) << ")";
        Util::endError();
        return false;
    }

    [[nodiscard]] const std::unordered_map<Control, bool, IO::ControlHash>& getControlMap() const { return m_controls; }

protected:
    std::unordered_map<Control, bool, IO::ControlHash> m_controls{};
};

class IOHandler : public EngineObject
{
public:
    explicit IOHandler(EngineObject* engine, GLFWwindow* window);

    // check whether we need to quit
    void update();

    // check if key has been pressed
    [[nodiscard]] bool getPressed(int key) const;
    // check if ESC has been pressed
    [[nodiscard]] bool getQuit() const { return m_quit; }

private:
    // for glfwGetKey
    GLFWwindow* m_window;

    bool m_quit{false};
};

#endif
