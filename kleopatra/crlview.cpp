/*
    crlview.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "crlview.h"

#include <klocale.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <KStandardGuiItem>
#include <kglobalsettings.h>

#include <QLayout>
#include <QLabel>
#include <qtextedit.h>
#include <QFontMetrics>
#include <QTimer>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCloseEvent>

CRLView::CRLView( QWidget* parent )
  : QDialog( parent ), _process(0)
{
  QVBoxLayout* topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( 4 );
  topLayout->setMargin( 10 );

  topLayout->addWidget( new QLabel( i18n("CRL cache dump:"), this ) );

  _textView = new QTextEdit( this );
  _textView->setFont( KGlobalSettings::fixedFont() );
  _textView->setReadOnly(true);
  topLayout->addWidget( _textView );

  QHBoxLayout* hbLayout = new QHBoxLayout();
  topLayout->addItem( hbLayout );

  _updateButton = new KPushButton( i18n("&Update"), this );
  _closeButton = new KPushButton( KStandardGuiItem::close(), this );

  hbLayout->addWidget( _updateButton );
  hbLayout->addStretch();
  hbLayout->addWidget( _closeButton );

  // connections:
  connect( _updateButton, SIGNAL(clicked()),
	   this, SLOT(slotUpdateView()) );
  connect( _closeButton, SIGNAL(clicked()),
	   this, SLOT(close()) );

  resize( _textView->fontMetrics().width( 'M' ) * 80,
	  _textView->fontMetrics().lineSpacing() * 25 );

  _timer = new QTimer( this );
  connect( _timer, SIGNAL(timeout()), SLOT(slotAppendBuffer()) );
}

CRLView::~CRLView()
{
  delete _process; _process = 0;
}

void CRLView::closeEvent( QCloseEvent * e ) {
  QDialog::closeEvent( e );
  delete _process; _process = 0;
}

void CRLView::slotUpdateView()
{
  _updateButton->setEnabled( false );
  _textView->clear();
  _buffer.clear();
  if( !_process ) {
    _process = new KProcess();
    *_process << "gpgsm" << "--call-dirmngr" << "listcrls";
    connect( _process, SIGNAL(readyReadStandardOutput()), this, SLOT(slotReadStdout()));
    connect( _process, SIGNAL(finished(int,QProcess::ExitStatus)),
             this, SLOT(slotProcessExited(int,QProcess::ExitStatus)));
  }
  if( _process->state() == QProcess::Running ) _process->kill();
  _process->setOutputChannelMode(KProcess::OnlyStdoutChannel);
  _process->start();
  if( !_process->waitForStarted()){
    KMessageBox::error( this, i18n( "Unable to start gpgsm process. Please check your installation." ), i18n( "Certificate Manager Error" ) );
    processExited();
   }
  _timer->start( 1000 );
}

void CRLView::slotReadStdout()
{
  _buffer.append( _process->readAllStandardOutput() );
}

void CRLView::slotAppendBuffer() {
  _textView->append( _buffer );
  _buffer.clear();
}

void CRLView::processExited()
{
  _timer->stop();
  slotAppendBuffer();
  _updateButton->setEnabled( true );
}


void CRLView::slotProcessExited(int, QProcess::ExitStatus _status)
{
  processExited();
  if( _status != QProcess::NormalExit ) {
    KMessageBox::error( this, i18n( "The GpgSM process ended prematurely because of an unexpected error." ), i18n( "Certificate Manager Error" ) );
  }
}

#include "crlview.moc"
