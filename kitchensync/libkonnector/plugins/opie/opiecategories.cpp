
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

}
OpieCategories &OpieCategories::operator=(const OpieCategories &op )
{
    m_name = op.m_name;
    m_app = op.m_app;
    m_id = op.m_id;
    return (*this);
}




