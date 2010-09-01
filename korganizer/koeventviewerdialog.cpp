/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "koeventviewerdialog.h"

#include "koeventviewer.h"

#include <klocale.h>

KOEventViewerDialog::KOEventViewerDialog( Calendar *calendar, TQWidget *parent,
                                          const char *name, bool compact )
  : KDialogBase( parent, name, false, i18n("Event Viewer"), Ok, Ok, false,
                 i18n("Edit") )
{
  mEventViewer = new KOEventViewer( calendar, this );
  setMainWidget( mEventViewer );

  if ( compact ) {
    setFixedSize( 240,284 );
    move( 0, 15 );
  } else {
    setMinimumSize( 500, 500 );
    resize( 520, 500 );
  }
  connect( this, TQT_SIGNAL(finished()), this, TQT_SLOT(delayedDestruct()) );
}

KOEventViewerDialog::~KOEventViewerDialog()
{
}

void KOEventViewerDialog::setCalendar( Calendar *calendar )
{
  mEventViewer->setCalendar( calendar );
}

void KOEventViewerDialog::addText( const TQString &text )
{
  mEventViewer->addText( text );
}

#include "koeventviewerdialog.moc"
