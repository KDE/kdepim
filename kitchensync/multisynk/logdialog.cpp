#include <klocale.h>
#include <libkdepim/progressmanager.h>

#include <qdatetime.h>
#include <qlayout.h>
#include <qtextedit.h>

#include "logdialog.h"

LogDialog::LogDialog( QWidget *parent )
  : KDialogBase( Plain, i18n( "Log Dialog" ), Ok | User1, Ok,
                 parent, "", false, true )
{
  initGUI();

  KPIM::ProgressManager *pm = KPIM::ProgressManager::instance();
  connect ( pm, SIGNAL( progressItemAdded( KPIM::ProgressItem* ) ),
            this, SLOT( progressItemAdded( KPIM::ProgressItem* ) ) );
  connect ( pm, SIGNAL( progressItemStatus( KPIM::ProgressItem*, const QString& ) ),
            this, SLOT( progressItemStatus( KPIM::ProgressItem*, const QString& ) ) );

  setButtonText( User1, i18n( "Clear Log" ) );

  connect( this, SIGNAL( user1Clicked() ),
           mView, SLOT( clear() ) );

  setInitialSize( QSize( 550, 260 ) );
}

void LogDialog::progressItemAdded( KPIM::ProgressItem *item )
{
  log( item->status() );
}

void LogDialog::progressItemStatus( KPIM::ProgressItem*, const QString &statusMsg )
{
  log( statusMsg );
}

void LogDialog::log( const QString &msg )
{
  mView->append( QDateTime::currentDateTime().toString( Qt::ISODate ) + ": " + msg + "\n" );
}

void LogDialog::initGUI()
{
  QWidget *page = plainPage();

  QVBoxLayout *layout = new QVBoxLayout( page, marginHint(), spacingHint() );
  mView = new QTextEdit( page );
  mView->setReadOnly( true );

  layout->addWidget( mView );
}

#include "logdialog.moc"
