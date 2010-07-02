#include "abstractdump.h"

#include <QtCore/QDir>
#include <kdebug.h>

AbstractDump::AbstractDump( const QDir &path, QObject *parent ) :
    QObject( parent ), m_path( path )
{
  if ( !m_path.exists() )
    kError() << "AbstractDump::AbstractDump(): path does not exist: " << m_path;
}

AbstractDump::~AbstractDump()
{
}

QDir AbstractDump::path() const
{
  return m_path;
}
