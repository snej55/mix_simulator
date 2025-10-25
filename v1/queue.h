#ifndef QUEUE_H
#define QUEUE_H

#include "engine.h"
#include "shader.h"
#include "objects.h"

#include <vector>
#include <utility> // for std::pair

class RenderQueue : EngineObject
{
public:
    explicit RenderQueue(EngineObject* parent);

    void addObject(const std::pair<Shader*, Object*>&& object);
    [[nodiscard]] const std::vector<std::pair<Shader*, Object*>>& getObjects() const;

    // gets last element from the stack
    [[nodiscard]] std::pair<Shader*, Object*> getNext() const;
    void popBack();

    [[nodiscard]] bool isEmpty() const;

private:
    std::vector<std::pair<Shader*, Object*>> m_objects{};
};

#endif