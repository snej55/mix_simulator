#ifndef SCENE_H
#define SCENE_H

#include "engine_types.hpp"

class SceneGraph final : public EngineObject
{
public:
    explicit SceneGraph(EngineObject* parent);
    ~SceneGraph() override;

    bool init(const char* scenePath);

private:
};

#endif // SCENE_H
