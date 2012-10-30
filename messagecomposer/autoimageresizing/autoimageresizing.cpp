/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "autoimageresizing.h"
#include <KLocale>

using namespace MessageComposer;

AutoImageResizing::AutoImageResizing(QWidget *parent)
    :KDialog(parent)
{
  setCaption( i18nc("@title:window", "Resize Image") );
  setButtons( User1 | Cancel );
  setDefaultButton( User1 );
  setModal( false );
  setButtonText( User1, i18nc("@action:button","Resize") );
  connect( this, SIGNAL(user1Clicked()), this, SLOT(slotUser1()) );
}

AutoImageResizing::~AutoImageResizing()
{
}

void AutoImageResizing::slotUser1()
{
  close();
}

#include "autoimageresizing.moc"
