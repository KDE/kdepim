
#include "ksync_profile.h"

using namespace KitchenSync;

Profile::Profile( const KDevice& dev,  const Kapabilities& caps,  const QString& name,  bool enable )
{
    m_device = dev;
    m_caps = caps;
    m_name = name;
    m_configured = enable;
}
Profile::Profile( const Profile& prof )
{
    (*this) = prof;
}
Profile::~Profile()
{

}
KDevice Profile::device() const
{
    return m_device;
}
QString Profile::name() const
{
    return m_name;
}
bool Profile::isConfigured() const
{
    return m_configured;
}
Kapabilities Profile::caps()const
{
    return m_caps;
}
Profile &Profile::operator=( const Profile &prof )
{
    m_name = prof.m_name;
    m_device = prof.m_device;
    m_configured = prof.m_configured;
    m_caps = prof.m_caps;
    return *this;
}
