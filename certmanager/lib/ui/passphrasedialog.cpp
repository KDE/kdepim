/*  -*- c++ -*-
    passphrasedialog.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
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


    Based on kpgpui.cpp
    Copyright (C) 2001,2002 the KPGP authors
    See file libkdenetwork/AUTHORS.kpgp for details

    This file is part of KPGP, the KDE PGP/GnuPG support library.

    KPGP is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
*/

#ifndef HAVE_CONFIG_H
#include <config.h>
#endif

#include "passphrasedialog.h"

#include <kpassdlg.h>
#include <kiconloader.h>
#include <klocale.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qfontmetrics.h>

struct Kleo::PassphraseDialog::Private {
  KPasswordEdit * lineedit;
};


Kleo::PassphraseDialog::PassphraseDialog( const QString & msg, const QString & caption,
					  QWidget * parent, const char * name, bool modal )
  : KDialogBase( parent, name, modal, caption, Ok|Cancel, Ok ), d( 0 )
{
  d = new Private();

  QWidget * w = new QWidget( this );
  setMainWidget( w );

  QHBoxLayout * hlay = new QHBoxLayout( w, 0, spacingHint() );

  QLabel * label = new QLabel( w );
  label->setPixmap( DesktopIcon( "pgp-keys", KIcon::SizeMedium ) );
  hlay->addWidget( label, 0, AlignTop );

  QVBoxLayout * vlay = new QVBoxLayout( hlay ); // inherits spacing

  vlay->addWidget( new QLabel( msg.isEmpty() ? i18n("Please enter your passphrase:") : msg, w ) );

  vlay->addWidget( new QLabel( i18n( "Enter passphrase:" ), w ) );
  d->lineedit = new KPasswordEdit( KPasswordEdit::OneStar, w, "d->lineedit" );
  d->lineedit->setMinimumWidth( fontMetrics().width("*") * 20 );
  d->lineedit->setFocus();

  vlay->addWidget( d->lineedit );

  connect( d->lineedit, SIGNAL(returnPressed()), SLOT(slotOk()) );

  disableResize();
}


Kleo::PassphraseDialog::~PassphraseDialog() {
  delete d; d = 0;
}

const char * Kleo::PassphraseDialog::passphrase() const {
  return d->lineedit->password();
}

void Kleo::PassphraseDialog::slotOk() {
  const char * pass = passphrase();
  emit finished( pass ? pass : "" );
  KDialogBase::slotOk();
}

void Kleo::PassphraseDialog::slotCancel() {
  emit canceled();
  KDialogBase::slotCancel();
}

  
void Kleo::PassphraseDialog::virtual_hook( int id, void * data ) {
  return KDialogBase::virtual_hook( id, data );
}

#include "passphrasedialog.moc"
