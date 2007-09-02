/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "devicewizard.h"

#include "firstpage.h"
#include "welcomepage.h"
#include "lastpage.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocalizedstring.h>
#include <kiconloader.h>

DeviceWizard::DeviceWizard( QWidget* parent )
    : QWizard( parent )
{
    setWindowTitle( i18nc( "new device wizard window title", "KMobileTools New Device Wizard" ) );

    // setting wizard pictures
    QPixmap wizardLogoPixmap;
    wizardLogoPixmap.load( KGlobal::dirs ()->findResource( "data", "kmobiletools/kmobilewizard.png" ) );
    setPixmap( QWizard::WatermarkPixmap, wizardLogoPixmap );

    // prepare pages
    QWizardPage* welcomePage = new WelcomePage( this );
    setPage( 0, welcomePage );

    QWizardPage* firstPage = new FirstPage(this);
    setPage( 1, firstPage );

    QWizardPage* lastPage = new LastPage( this );
    setPage( 0xFFFF, lastPage );
}

DeviceWizard::~DeviceWizard() {

}

#include "devicewizard.moc"
