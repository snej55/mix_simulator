#ifndef SCENE_H
#define SCENE_H

#include "engine_types.hpp"

class Scene final : public EngineObject
{
public:
    explicit Scene(EngineObject* parent);
    ~Scene() override;

    bool init(const char* scenePath);

private:
};

#endif // SCENE_H
