#include <rmm/Entity.h>

using namespace RMM;

Entity::Entity()
    :   MessageComponent()
{
    // Empty.
}

Entity::~Entity()
{
}

Entity::Entity(const Entity & h)
    :   MessageComponent(h)
{
}

Entity::Entity(const QCString & s)
    :   MessageComponent(s)
{
    // Empty.
}

    Entity &
Entity::operator = (const QCString & s)
{
    MessageComponent::operator = (s);
    return *this;
}

    Entity &
Entity::operator = (const Entity & h)
{
    if (this == &h) return *this;
    MessageComponent::operator = (h);
    return *this;
}

    bool
Entity::operator == (Entity &)
{
    return false;
}

    void
Entity::_parse()
{
}

    void
Entity::_assemble()
{
}

    void
Entity::createDefault()
{
}

// vim:ts=4:sw=4:tw=78
