/*
 * Basic memory manager
 * Process:
 * EngineObject* object;
 * // allocate memory for object
 * EngineObject* object = new EngineObject{"foo", nullptr};
 * arena.addObject(object);
 */

#ifndef ARENA_H
#define ARENA_H

#include <vector>

#include "engine_types.hpp"

class Arena : public EngineObject
{
public:
    // set up arena
    explicit Arena(EngineObject* engine);

    ~Arena() override;
    // add object to arena and update object ID
    void addObject(EngineObject* object);
    // remove object from arena
    void removeObject(unsigned int id);

private:
    std::vector<EngineObject*> m_objects{};

    // free function should only be called from destructor
    void free();
};

#endif
