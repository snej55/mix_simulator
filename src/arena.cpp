#include "arena.hpp"
#include "util.hpp"

#include <iostream>

Arena::Arena(EngineObject* engine) : EngineObject{"Arena", engine} {}

Arena::~Arena() { free(); }

// free memory of objects
void Arena::free()
{
    for (std::size_t i{0}; i < m_objects.size(); ++i)
    {
        delete m_objects[i];
    }
    m_objects.clear();
}

// add new object to the arena
void Arena::addObject(EngineObject* object)
{
    object->setID(m_objects.size());
    m_objects.emplace_back(object);
}

// remove object from arena
void Arena::removeObject(const unsigned int id)
{
    if (id > m_objects.size() - 1)
    {
        Util::beginError();
        std::cout << "ARENA::REMOVE_OBJECT::ERROR: Index out of range!";
        Util::endError();
        return;
    }
    // move object to back
    std::swap(m_objects[id], m_objects.back());
    // update id of other moved object
    m_objects[id]->setID(id);

    // free object memory
    delete m_objects.back();
    m_objects.pop_back();
}
