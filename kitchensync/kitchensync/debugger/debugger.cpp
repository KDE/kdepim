#include "debugger.h"

#include <konnectormanager.h>
#include <konnectorinfo.h>
#include <mainwindow.h>

#include <kaboutdata.h>
#include <kiconloader.h>
#include <kparts/genericfactory.h>

#include <qlabel.h>

typedef KParts::GenericFactory< KSync::Debugger> DebuggerFactory;
K_EXPORT_COMPONENT_FACTORY( libksync_debugger, DebuggerFactory );

using namespace KSync ;

Debugger::Debugger( QWidget *parent, const char *name,
                    QObject *, const char *,const QStringList & )
  : ManipulatorPart( parent, name ), m_widget( 0 )
{
  m_pixmap = KGlobal::iconLoader()->loadIcon("kcmsystem", KIcon::Desktop, 48 );
}

KAboutData *Debugger::createAboutData()
{
  return new KAboutData("KSyncDebugger",  I18N_NOOP("Sync Debugger Part"), "0.0" );
}

Debugger::~Debugger()
{
  delete m_widget;
}

QString Debugger::type() const
{
  return QString::fromLatin1("debugger");
}

QString Debugger::name() const
{
  return i18n("Konnector Debugger");
}

QString Debugger::description() const
{
  return i18n("Debugger for Konnectors");
}

QPixmap *Debugger::pixmap()
{
  return &m_pixmap;
}

QString Debugger::iconName() const
{
  return QString::fromLatin1("kcmsystem");
}

bool Debugger::partIsVisible() const
{
  return true;
}

QWidget* Debugger::widget()
{
  if( !m_widget ) {
    m_widget = new QLabel( "Hallo", 0 );
  }
  return m_widget;
}

#include "debugger.moc"
