
#include "opiecategories.h"

OpieCategories::OpieCategories()
{

}
OpieCategories::OpieCategories(const QString &id, const QString &name, const QString &app )
{
    m_name = name;
    m_id = id;
    m_app = app;
}
OpieCategories::OpieCategories(const OpieCategories &op )
{
    (*this) = op;
}
QString OpieCategories::id() const
{
    return m_id;
}
QString OpieCategories::name() const
{
    return m_name;
}
QString OpieCategories::app() const
{
    return m_app;
}
OpieCategories &OpieCategories::operator=(const OpieCategories &op )
{
    m_name = op.m_name;
    m_app = op.m_app;
    m_id = op.m_id;
    return (*this);
}


bool operator== (const OpieCategories& a,  const OpieCategories &b )
{
    if ( a.id() == b.id() && a.name() == b.name() && a.app() == b.app() )
        return true;
    return false;
}





