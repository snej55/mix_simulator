#ifndef RENDERER_H
#define RENDERER_H

#include "engine.h"
#include "queue.h"
#include "objects.h"

class Renderer : public EngineObject
{
public:
    explicit Renderer(EngineObject* parent);
    ~Renderer() override;

    virtual bool init();
    void free() override;

    virtual bool createRenderQueue();

    // clears the queue and renders all the objects from it
    virtual void renderQueue();
    virtual void drawObject(const Shader* shader, Object* object);
    virtual void addObject(Shader* shader, Object* object);

protected:
    RenderQueue* m_renderQueue{nullptr};
};

#endif