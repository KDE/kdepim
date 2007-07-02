#include "configguimoto.h"

#include <qdom.h>
#include <qlabel.h>
#include <qlayout.h>

#include <klineedit.h>
#include <kdialog.h>
#include <klocale.h>

ConfigGuiMoto::ConfigGuiMoto( const QSync::Member &member, QWidget *parent )
  : ConfigGui( member, parent )
{
  initGUI();
}

void ConfigGuiMoto::load( const QString &xml )
{
  QDomDocument doc;
  doc.setContent( xml );
  QDomElement docElement = doc.documentElement();
  QDomNode node;
  for( node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    QDomElement element = node.toElement();
    if ( element.tagName() == "device" ) {
      mDeviceString->setText( element.text() );
    }
  }
}

QString ConfigGuiMoto::save()
{
  QString config = "<config>\n";

  config += QString( "<device>%1</device>\n" ).arg( mDeviceString->text() );

  config += "</config>";

  return config;
}

void ConfigGuiMoto::initGUI()
{
  QGridLayout *layout = new QGridLayout( topLayout(), 12, 3, KDialog::spacingHint() );
  layout->setMargin( KDialog::marginHint() );

  layout->addWidget( new QLabel( i18n( "Device String:" ), this ), 0, 0 );
  mDeviceString = new KLineEdit( this );
  layout->addMultiCellWidget( mDeviceString, 0, 0, 1, 2 );
}
