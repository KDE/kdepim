
#include <qobject.h>
#include <qwidget.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kgenericfactory.h>
#include <kparts/componentfactory.h>

#include "ksync_fileviewpart.h"
#include "ksync_fileviewwidget.h"

typedef KGenericFactory< KitchenSync::FileviewPart> FileviewPartFactory;
K_EXPORT_COMPONENT_FACTORY( libfileviewpart, FileviewPartFactory )

using namespace KitchenSync ;

FileviewPart::FileviewPart(QWidget *parent, const char *name,
                           QObject *obj, const char *na, const QStringList & )
    : KitchenSync::ManipulatorPart( parent, name ) {
    //setInstance(FileviewPartFactory::instance() );
    m_pixmap = KGlobal::iconLoader()->loadIcon("konqueror", KIcon::Desktop, 48 );
    m_widget = 0;
    m_config = 0;
}
FileviewPart::~FileviewPart() {
}

QPixmap* FileviewPart::pixmap() {
  return &m_pixmap;
}

QWidget* FileviewPart::widget() {
    if(m_widget == 0 ) {
    m_widget = new KSyncFileviewWidget();
  }
  return m_widget;
}

QWidget* FileviewPart::configWidget() {
    kdDebug(5223) << "configWidget \n" ;
    m_config = new QWidget();
    m_config->setBackgroundColor( Qt::blue );
    return m_config;
}

#include "ksync_fileviewpart.moc"
#include "fileviewpart.moc"
