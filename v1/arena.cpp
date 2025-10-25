//
// Created by Jens Kromdijk on 13/05/25.
//

#include "arena.h"

#include <cassert>

Arena::Arena( const unsigned int capacity)
{
    assert(capacity > 0); // capacity of 0 is useless
    m_components.reserve(capacity); // reserve space for components, default is 10
}

Arena::~Arena()
{
    free();
}

// clear object pool
void Arena::free()
{
    if (!m_freed)
    {
        for (std::size_t i{0}; i < std::size(m_components); ++i)
        {
            delete m_components[i];
            m_components[i] = nullptr;
        }
        m_freed = true;
    }
}

template <typename T>
std::pair<T*, unsigned int> Arena::addObject()
{
    // allocate memory for object
    const T* ptr {new T};

    return std::pair<T*, unsigned int>{ptr, m_components.size()};
}