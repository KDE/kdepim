#include <qlayout.h>
#include <qlabel.h>
#include <qtextview.h>
#include <qpushbutton.h>

#include <klocale.h>
#include <kprocess.h>
#include <kmessagebox.h>

#include "crlview.h"

CRLView::CRLView( QWidget* parent, const char* name, bool modal )
  : QDialog( parent, name, modal ), _process(0)
{
  QVBoxLayout* topLayout = new QVBoxLayout( this, 10, 4 );

  topLayout->addWidget( new QLabel( i18n("CRL cache dump:"), this ) );

  _textView = new QTextView( this );
  _textView->setWordWrap( QTextEdit::NoWrap );
  topLayout->addWidget( _textView );
  
  QHBoxLayout* hbLayout = new QHBoxLayout( topLayout );
  
  _updateButton = new QPushButton( i18n("&Update"), this );
  _closeButton = new QPushButton( i18n("&Close"), this );
  
  hbLayout->addWidget( _updateButton );
  hbLayout->addStretch();
  hbLayout->addWidget( _closeButton );  

  // connections:
  connect( _updateButton, SIGNAL( clicked() ),
	   this, SLOT( slotUpdateView() ) );
  connect( _closeButton, SIGNAL( clicked() ),
	   this, SLOT( close() ) );
}

CRLView::~CRLView()
{
  delete _process;
}

void CRLView::slotUpdateView()
{
  _updateButton->setEnabled( false );
  _textView->clear();
  if( _process == 0 ) {
    _process = new KProcess();
    *_process << "dirmngr";
    *_process << "--list-crls";
    connect( _process, SIGNAL( receivedStdout( KProcess*, char*, int) ),
	     this, SLOT( slotReadStdout( KProcess*, char*, int ) ) );
    connect( _process, SIGNAL( processExited( KProcess* ) ),
	     this, SLOT( slotProcessExited() ) );
  }
  if( _process->isRunning() ) _process->kill();
  if( !_process->start( KProcess::NotifyOnExit, KProcess::Stdout ) ) {
    KMessageBox::error( this, i18n( "Unable to start dirmngr process. Please check your installation." ), i18n( "Certificate Manager Error" ) );
    slotProcessExited();
  }
}

void CRLView::slotReadStdout( KProcess*, char* buf, int len)
{
  _textView->append( QString::fromUtf8( buf, len ) );
}

void CRLView::slotProcessExited()
{
  _updateButton->setEnabled( true );

  if( !_process->normalExit() ) {
    KMessageBox::error( this, i18n( "The Dirmngr process ended prematurely because of an unexpected error." ), i18n( "Certificate Manager Error" ) );
  }
}
