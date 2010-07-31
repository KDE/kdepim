/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "freebusyurldialog.h"

#include <libkcal/attendee.h>
#include <libkcal/freebusyurlstore.h>

#include <klineedit.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include <tqlayout.h>
#include <tqlabel.h>

FreeBusyUrlDialog::FreeBusyUrlDialog( KCal::Attendee *attendee, TQWidget *parent,
                                      const char *name )
  : KDialogBase( Plain, i18n("Edit Free/Busy Location"), Ok|Cancel, Ok, parent,
                 name, true, false )
{
  TQFrame *topFrame = plainPage();

  TQBoxLayout *topLayout = new TQVBoxLayout( topFrame, 0, spacingHint() );

  mWidget = new FreeBusyUrlWidget( attendee, topFrame );
  topLayout->addWidget( mWidget );

  mWidget->loadConfig();
}

void FreeBusyUrlDialog::slotOk()
{
  mWidget->saveConfig();
  accept();
}


FreeBusyUrlWidget::FreeBusyUrlWidget( KCal::Attendee *attendee, TQWidget *parent,
                                      const char *name )
  : TQWidget( parent, name ), mAttendee( attendee )
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  
  TQLabel *label = new TQLabel(
      i18n("Location of Free/Busy information for %1 <%2>:")
      .arg( mAttendee->name() ).arg( mAttendee->email() ), this );
  topLayout->addWidget( label );

  mUrlEdit = new KLineEdit( this );
  topLayout->addWidget( mUrlEdit );
}

FreeBusyUrlWidget::~FreeBusyUrlWidget()
{
}

void FreeBusyUrlWidget::loadConfig()
{
  kdDebug(5850) << "FreeBusyUrlWidget::loadConfig()" << endl;

  TQString url = KCal::FreeBusyUrlStore::self()->readUrl( mAttendee->email() );

  mUrlEdit->setText( url );
}

void FreeBusyUrlWidget::saveConfig()
{
  kdDebug(5850) << "FreeBusyUrlWidget::saveConfig()" << endl;

  TQString url = mUrlEdit->text();
  
  KCal::FreeBusyUrlStore::self()->writeUrl( mAttendee->email(), url );

  KCal::FreeBusyUrlStore::self()->sync();
}

#include "freebusyurldialog.moc"
