
#include "opiecategories.h"

OpieCategories::OpieCategories()
{
    m_id = 0;
}
OpieCategories::OpieCategories(int id, const QString &name, const QString &app )
{
    m_name = name;
    m_id = id;
    m_app = app;
}
OpieCategories::OpieCategories(const OpieCategories &op )
{
    (*this) = op;
}
OpieCategories &OpieCategories::operator=(const OpieCategories &op )
{
    m_name = op.m_name;
    m_app = op.m_app;
    m_id = op.m_id;
    return (*this);
}
