#include "abstractdump.h"

#include <QtCore/QDir>
#include <kdebug.h>

AbstractDump::AbstractDump( QString path, QObject *parent ) :
    QObject(parent)
{
  QDir dir(path);
  m_path = dir.absolutePath();
  if ( !dir.exists() )
    kError() << "AbstractDump::AbstractDump(): path does not exist: " << m_path;
}

AbstractDump::~AbstractDump()
{
}

QString AbstractDump::path() const
{
  return m_path;
}
