#ifndef IO_HANDLER_H
#define IO_HANDLER_H

#include "engine.h"

class IOHandler : public EngineObject
{
public:
    explicit IOHandler(EngineObject* window);

    // check whether we need to quit
    virtual void update();

    [[nodiscard]] bool getPressed(int key) const;
    [[nodiscard]] bool getQuit() const {return m_quit;}

private:
    bool m_quit{false};
    EngineObject* m_window{nullptr};
};

#endif