#include "configguisynce.h"

#include <qdom.h>
#include <qlabel.h>
#include <qlayout.h>

#include <klineedit.h>
#include <kdialog.h>
#include <klocale.h>

ConfigGuiSynce::ConfigGuiSynce( const QSync::Member &member, QWidget *parent )
  : ConfigGui( member, parent )
{
  initGUI();
}

void ConfigGuiSynce::load( const QString &xml )
{
  QDomDocument doc;
  doc.setContent( xml );
  QDomElement docElement = doc.documentElement();
  QDomNode node;
  for( node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    QDomElement element = node.toElement();
    if ( element.tagName() == "contact" ) {
      mContacts->setText( element.text() );
    } else if ( element.tagName() == "todos" ) {
      mTodos->setText( element.text() );
    } else if ( element.tagName() == "calendar" ) {
      mCalendar->setText( element.text() );
    } else if ( element.tagName() == "file" ) {
      mFile->setText( element.text() );
    }
  }
}

QString ConfigGuiSynce::save()
{
  QString config = "<config>\n";

  config += QString( "<contact>%1</contact>\n" ).arg( mContacts->text() );
  config += QString( "<todos>%1</todos>\n" ).arg( mTodos->text() );
  config += QString( "<calendar>%1</calendar>\n" ).arg( mCalendar->text() );
  config += QString( "<file>%1</file>\n" ).arg( mFile->text() );

  config += "</config>";

  return config;
}

void ConfigGuiSynce::initGUI()
{
  QGridLayout *layout = new QGridLayout( topLayout(), 12, 4, KDialog::spacingHint() );
  layout->setMargin( KDialog::marginHint() );

  layout->addWidget( new QLabel( i18n( "Contacts:" ), this ), 0, 0 );
  mContacts = new KLineEdit( this );
  layout->addMultiCellWidget( mContacts, 0, 0, 1, 2 );

  layout->addWidget( new QLabel( i18n( "Todo List:" ), this ), 1, 0 );
  mTodos = new KLineEdit( this );
  layout->addMultiCellWidget( mTodos, 1, 1, 1, 2 );

  layout->addWidget( new QLabel( i18n( "Calendar:" ), this ), 2, 0 );
  mCalendar = new KLineEdit( this );
  layout->addMultiCellWidget( mCalendar, 2, 2, 1, 2 );

  layout->addWidget( new QLabel( i18n( "File:" ), this ), 3, 0 );
  mFile = new KLineEdit( this );
  layout->addMultiCellWidget( mFile, 3, 3, 1, 2 );
}
