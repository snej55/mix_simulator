#include "queue.h"

RenderQueue::RenderQueue(EngineObject* parent)
 : EngineObject{parent, "RenderQueue"}
{
}

void RenderQueue::addObject(const std::pair<Shader*, Object*>&& object)
{
    m_objects.push_back(object);
}

const std::vector<std::pair<Shader*, Object*>>& RenderQueue::getObjects() const
{
    return m_objects;
}

// returns last item from the stack
std::pair<Shader*, Object*> RenderQueue::getNext() const
{
    return m_objects.back();
}

void RenderQueue::popBack()
{
    m_objects.pop_back();
}

bool RenderQueue::isEmpty() const
{
    return m_objects.empty();
}