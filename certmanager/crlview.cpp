/*
    crlview.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klar�lvdalens Datakonsult AB

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#include <qlayout.h>
#include <qlabel.h>
#include <qtextview.h>

#include <klocale.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

#include "crlview.h"
#include "crlview.moc"

CRLView::CRLView( QWidget* parent, const char* name, bool modal )
  : QDialog( parent, name, modal ), _process(0)
{
  QVBoxLayout* topLayout = new QVBoxLayout( this, 10, 4 );

  topLayout->addWidget( new QLabel( i18n("CRL cache dump:"), this ) );

  _textView = new QTextView( this );
  _textView->setWordWrap( QTextEdit::NoWrap );
  topLayout->addWidget( _textView );

  QHBoxLayout* hbLayout = new QHBoxLayout( topLayout );

  _updateButton = new KPushButton( i18n("&Update"), this );
  _closeButton = new KPushButton( KStdGuiItem::close(), this );

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
    *_process << "gpgsm" << "--call-dirmngr" << "listcrls";
    connect( _process, SIGNAL( receivedStdout( KProcess*, char*, int) ),
	     this, SLOT( slotReadStdout( KProcess*, char*, int ) ) );
    connect( _process, SIGNAL( processExited( KProcess* ) ),
	     this, SLOT( slotProcessExited() ) );
  }
  if( _process->isRunning() ) _process->kill();
  if( !_process->start( KProcess::NotifyOnExit, KProcess::Stdout ) ) {
    KMessageBox::error( this, i18n( "Unable to start gpgsm process. Please check your installation." ), i18n( "Certificate Manager Error" ) );
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
    KMessageBox::error( this, i18n( "The GpgSM process ended prematurely because of an unexpected error." ), i18n( "Certificate Manager Error" ) );
  }
}
