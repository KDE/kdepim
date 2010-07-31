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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "crlview.h"

#include <klocale.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kglobalsettings.h>

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqtextedit.h>
#include <tqfontmetrics.h>
#include <tqtimer.h>

CRLView::CRLView( TQWidget* parent, const char* name, bool modal )
  : TQDialog( parent, name, modal ), _process(0)
{
  TQVBoxLayout* topLayout = new TQVBoxLayout( this, 10, 4 );

  topLayout->addWidget( new TQLabel( i18n("CRL cache dump:"), this ) );

  _textView = new TQTextEdit( this );
  _textView->setFont( KGlobalSettings::fixedFont() );
  _textView->setTextFormat( TQTextEdit::LogText );
  topLayout->addWidget( _textView );

  TQHBoxLayout* hbLayout = new TQHBoxLayout( topLayout );

  _updateButton = new KPushButton( i18n("&Update"), this );
  _closeButton = new KPushButton( KStdGuiItem::close(), this );

  hbLayout->addWidget( _updateButton );
  hbLayout->addStretch();
  hbLayout->addWidget( _closeButton );

  // connections:
  connect( _updateButton, TQT_SIGNAL( clicked() ),
	   this, TQT_SLOT( slotUpdateView() ) );
  connect( _closeButton, TQT_SIGNAL( clicked() ),
	   this, TQT_SLOT( close() ) );

  resize( _textView->fontMetrics().width( 'M' ) * 80,
	  _textView->fontMetrics().lineSpacing() * 25 );

  _timer = new TQTimer( this );
  connect( _timer, TQT_SIGNAL(timeout()), TQT_SLOT(slotAppendBuffer()) );
}

CRLView::~CRLView()
{
  delete _process; _process = 0;
}

void CRLView::closeEvent( TQCloseEvent * e ) {
  TQDialog::closeEvent( e );
  delete _process; _process = 0;
}

void CRLView::slotUpdateView()
{
  _updateButton->setEnabled( false );
  _textView->clear();
  _buffer = TQString::null;
  if( _process == 0 ) {
    _process = new KProcess();
    *_process << "gpgsm" << "--call-dirmngr" << "listcrls";
    connect( _process, TQT_SIGNAL( receivedStdout( KProcess*, char*, int) ),
	     this, TQT_SLOT( slotReadStdout( KProcess*, char*, int ) ) );
    connect( _process, TQT_SIGNAL( processExited( KProcess* ) ),
	     this, TQT_SLOT( slotProcessExited() ) );
  }
  if( _process->isRunning() ) _process->kill();
  if( !_process->start( KProcess::NotifyOnExit, KProcess::Stdout ) ) {
    KMessageBox::error( this, i18n( "Unable to start gpgsm process. Please check your installation." ), i18n( "Certificate Manager Error" ) );
    slotProcessExited();
  }
  _timer->start( 1000 );
}

void CRLView::slotReadStdout( KProcess*, char* buf, int len)
{
  _buffer.append( TQString::fromUtf8( buf, len ) );
}

void CRLView::slotAppendBuffer() {
  _textView->append( _buffer );
  _buffer = TQString::null;
}

void CRLView::slotProcessExited()
{
  _timer->stop();
  slotAppendBuffer();
  _updateButton->setEnabled( true );

  if( !_process->normalExit() ) {
    KMessageBox::error( this, i18n( "The GpgSM process ended prematurely because of an unexpected error." ), i18n( "Certificate Manager Error" ) );
  }
}

#include "crlview.moc"
