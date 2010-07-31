/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <tqlabel.h>
#include <tqstringlist.h>
#include <tqlayout.h>
#include <tqcombobox.h>

#include <kdebug.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

#include "statusdialog.h"
#include "statusdialog.moc"

StatusDialog::StatusDialog(TQWidget* parent, const char* name) :
  KDialog(parent,name,true)
{
  setCaption(i18n("Set Your Status"));

  TQBoxLayout *topLayout = new TQVBoxLayout( this );
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( marginHint() );

  TQBoxLayout *statusLayout = new TQHBoxLayout( topLayout );

  TQLabel *text = new TQLabel(i18n("Set your status"),this);
  statusLayout->addWidget( text );

  mStatus = new TQComboBox(false,this);
  mStatus->insertStringList(Attendee::statusList());
  statusLayout->addWidget( mStatus );

  TQBoxLayout *buttonLayout = new TQHBoxLayout( topLayout );

  TQPushButton *ok = new KPushButton(KStdGuiItem::ok(), this);
  connect ( ok,TQT_SIGNAL(clicked()), this,TQT_SLOT(accept()) );
  buttonLayout->addWidget( ok );

  TQPushButton *cancel = new KPushButton(KStdGuiItem::cancel(), this);
  connect ( cancel,TQT_SIGNAL(clicked()), this,TQT_SLOT(reject()) );
  buttonLayout->addWidget( cancel );
}

StatusDialog::~StatusDialog()
{
}

Attendee::PartStat StatusDialog::status()
{
  return Attendee::PartStat( mStatus->currentItem() ) ;
}
