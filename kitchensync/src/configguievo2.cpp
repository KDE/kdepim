#include "configguievo2.h"

#include <qdom.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qstring.h>

#include <kurlrequester.h>
#include <kurl.h>
#include <kfile.h>
#include <kdialog.h>
#include <klocale.h>

ConfigGuiEvo2::ConfigGuiEvo2( const QSync::Member &member, QWidget *parent )
  : ConfigGui( member, parent )
{
  initGUI();
}

void ConfigGuiEvo2::load( const QString &xml )
{
  QDomDocument doc;
  doc.setContent( xml );
  QDomElement docElement = doc.documentElement();
  QDomNode node;
  for( node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    QDomElement element = node.toElement();
    if ( element.tagName() == "address_path" ) {
      mAddressPath->setURL( element.text() );
    } else if ( element.tagName() == "calendar_path" ) {
      mCalendarPath->setURL( element.text() ) ;
    } else if ( element.tagName() == "tasks_path" ) {
      mTasksPath->setURL( element.text() );
    }
  }
}

QString ConfigGuiEvo2::save()
{
  QString config = "<config>\n";

  config += QString( "<address_path>%1</address_path>\n" ).arg( mAddressPath->url() );
  config += QString( "<calendar_path>%1</calendar_path>\n" ).arg( mCalendarPath->url() );
  config += QString( "<tasks_path>%1</tasks_path>\n" ).arg( mTasksPath->url() );

  config += "</config>";

  return config;
}

void ConfigGuiEvo2::initGUI()
{
  QGridLayout *layout = new QGridLayout( topLayout(), 12, 3, KDialog::spacingHint() );
  layout->setMargin( KDialog::marginHint() );

  layout->addWidget( new QLabel( i18n( "Address Book location:" ), this ), 0, 0 );
  mAddressPath = new KURLRequester( this );
  mAddressPath->setMode( KFile::Directory );
  layout->addMultiCellWidget( mAddressPath, 0, 0, 1, 2 );

  layout->addWidget( new QLabel( i18n( "Calendar location:" ), this ), 1, 0 );
  mCalendarPath = new KURLRequester( this );
  mCalendarPath->setMode( KFile::Directory );
  layout->addMultiCellWidget( mCalendarPath, 1, 1, 1, 2 );

  layout->addWidget( new QLabel( i18n( "Task list location:" ), this ), 2, 0 );
  mTasksPath = new KURLRequester( this );
  mTasksPath->setMode( KFile::Directory );
  layout->addMultiCellWidget( mTasksPath, 2, 2, 1, 2 );
}
