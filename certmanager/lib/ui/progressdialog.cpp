/*  -*- mode: C++; c-file-style: "gnu" -*-
    progressdialog.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

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
*/

#include "progressdialog.h"

#include "progressbar.h"

#include <kleo/job.h>

#include <kdebug.h>
#include <klocale.h>

#include <qtimer.h>

#include <assert.h>

Kleo::ProgressDialog::ProgressDialog( Job * job, const QString & baseText,
				      QWidget * creator, const char * name, WFlags f )
  : QProgressDialog( creator, name, false, f ), mBaseText( baseText )
{
  assert( job );
  setBar( new ProgressBar( this, "replacement progressbar in Kleo::ProgressDialog" ) );

  setMinimumDuration( 2000 /*ms*/ );
  setAutoReset( false );
  setAutoClose( false );
  setLabelText( baseText );
  setTotalSteps( 0 );

  connect( job, SIGNAL(progress(const QString&,int,int,int)),
	   SLOT(slotProgress(const QString&,int,int,int)) );
  connect( job, SIGNAL(done()), SLOT(slotDone()) );
  connect( this, SIGNAL(canceled()),
	   job, SLOT(slotCancel()) );

  QTimer::singleShot( minimumDuration(), this, SLOT(forceShow()) );
}

Kleo::ProgressDialog::~ProgressDialog() {

}

void Kleo::ProgressDialog::setMinimumDuration( int ms ) {
  if ( 0 < ms && ms < minimumDuration() )
    QTimer::singleShot( ms, this, SLOT(forceShow()) );
  QProgressDialog::setMinimumDuration( ms );
}

void Kleo::ProgressDialog::slotProgress( const QString & what, int type, int current, int total ) {
  kdDebug() << "Kleo::ProgressDialog::slotProgress( \"" << what << "\", "
	    << type << ", " << current << ", " << total << " )" << endl;
  if ( mBaseText.isEmpty() )
    setLabelText( what );
  else if ( what.isEmpty() )
    setLabelText( mBaseText );
  else
    setLabelText( i18n( "%1: %2" ).arg( mBaseText, what ) );
  setProgress( current, total );
}

void Kleo::ProgressDialog::slotDone() {
  kdDebug() << "Kleo::ProgressDialog::slotDone()" << endl;
  hide();
  deleteLater();
}


#include "progressdialog.moc"

