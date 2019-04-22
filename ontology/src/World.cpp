// ----------------------------------------------------------------------------
// World.cpp
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// include files

#include <ontology/World.hpp>
#include <ontology/EntityManager.hpp>
#include <ontology/SystemManager.hpp>
#include <ontology/Entity.hpp>

namespace ontology {

static ID GUIDCounter = ID(0);

// ----------------------------------------------------------------------------
World::World() :
    m_EntityManager(new EntityManager(this)),
    m_SystemManager(new SystemManager(this))
{
}

// ----------------------------------------------------------------------------
World::~World()
{
}

// ----------------------------------------------------------------------------
EntityManager& World::getEntityManager() const
{
    return *m_EntityManager.get();
}

// ----------------------------------------------------------------------------
SystemManager& World::getSystemManager() const
{
    return *m_SystemManager.get();
}

// ----------------------------------------------------------------------------
ID World::generateGUID()
{
    return ID(GUIDCounter.value++);
}

// ----------------------------------------------------------------------------
void World::update()
{
    m_SystemManager->update();
}

} // namespace ontology
