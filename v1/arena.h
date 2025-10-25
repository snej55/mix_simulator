//
// Created by Jens Kromdijk on 13/05/25.
//

/*
 * EngineObject* pool to handle component memory allocation and cleanup
 */

#ifndef ARENA_H
#define ARENA_H

#include <vector>
#include <utility>

#include "engine.h"

class Arena {
public:
    explicit Arena(unsigned int capacity = 10);
    ~Arena();

    void free();

    // returns memory address and object ID in pool
    template <typename T>
    std::pair<T*, unsigned int> addObject();

    // removes object from pool
    void killObject(unsigned int id);

    [[nodiscard]] const std::vector<EngineObject*>& getComponents() const {return m_components;}

    [[nodiscard]] unsigned int getCapacity() const {return m_components.capacity();}
    [[nodiscard]] unsigned int getSize() const {return m_components.size();}
    [[nodiscard]] bool getFreed() const {return m_freed;}

private:
    std::vector<EngineObject*> m_components;

    bool m_freed{false};
};



#endif //ARENA_H
