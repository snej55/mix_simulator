#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <string>
#include <iostream>

// base class for engine objects
class EngineObject
{
public:
    EngineObject(EngineObject* parent, const char* name, const std::vector<EngineObject*>& children = {})
     : m_parent{parent}, m_name{name}, m_children{children}
    {
        if (parent != nullptr)
        {
            // add self as node
            parent->addChild(this);
        }
        std::cout << "Created {" << name << "} as child of {" << (parent == nullptr ? "NULL" : parent->getName()) << "}\n";
    }

    virtual ~EngineObject()
    {
        if (!m_freed)
        {
            EngineObject::free();
        }
    }

    virtual void free()
    {
        for (EngineObject*& e: m_children)
        {
            if (e != nullptr)
            {
                e->free();
                const EngineObject* parent {e->getParent()};
                std::cout << "Freed {" << e->getName() << "}, child of {" << (parent == nullptr ? "NULL" : parent->getName()) << "}\n";
                delete e;
                e = nullptr;
            }
        }
        m_freed = true;
    }

    [[nodiscard]] virtual bool getFreed() const {return m_freed;}

    template <typename T>
    T* getParent() const
    {
        return static_cast<T*>(m_parent);
    }

    [[nodiscard]] virtual EngineObject* getParent() const
    {
        return m_parent;
    }
    
    virtual void clearParent() {m_parent = nullptr;}
    virtual void setParent(EngineObject* parent) {m_parent = parent;}

    [[nodiscard]] virtual std::vector<EngineObject*> getChildren() const
    {
        return m_children;
    }

    virtual void addChild(EngineObject* child)
    {
        m_children.push_back(child);
    }

    [[nodiscard]] virtual std::string_view getName() const
    {
        return m_name;
    }

protected:
    EngineObject* m_parent;
    std::string m_name;
    std::vector<EngineObject*> m_children;

    bool m_freed{false};
};

#endif