
#include <qdatetime.h>

#include "opiedesktopsyncentry.h"


OpieDesktopSyncEntry::OpieDesktopSyncEntry()
{

}
OpieDesktopSyncEntry::OpieDesktopSyncEntry(const QString &category, const QString &file, const QString &name, const QString &type, const QString &size  )
{
    m_category = category;
    m_file = file;
    m_name = name;
    m_type = type;
    m_size = size;
}
OpieDesktopSyncEntry::~OpieDesktopSyncEntry()
{

}
QString OpieDesktopSyncEntry::name()
{
    return m_name;
}
QString OpieDesktopSyncEntry::file() const
{
    return m_file;
}
QString OpieDesktopSyncEntry::type() const
{
    return m_type;
}
QString OpieDesktopSyncEntry::size() const
{
    return m_size;
}
QString OpieDesktopSyncEntry::id()
{
    return QString::null;
}
void OpieDesktopSyncEntry::setId(const QString & )
{

}
QString OpieDesktopSyncEntry::oldId()
{
    return QString::null;
}
void OpieDesktopSyncEntry::setOldId(const QString & )
{

}

QString OpieDesktopSyncEntry::timestamp()
{
    return QDate::currentDate().toString();
}
void OpieDesktopSyncEntry::setTimestamp(const QString & )
{
    
}
bool OpieDesktopSyncEntry::equals(KSyncEntry *entr )
{
    if( entr->type() == QString::fromLatin1("OpieDesktopEntry") ){
    OpieDesktopSyncEntry *entry = (OpieDesktopSyncEntry*)entr;
	if( m_name == entry->m_name && m_id == entry->m_id && m_category == entry->m_category && m_file == entry->m_file)
	    return true;
    }	
    return false;
}

