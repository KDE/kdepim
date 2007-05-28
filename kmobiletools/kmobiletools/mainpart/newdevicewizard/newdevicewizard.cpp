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

#include "newdevicewizard.h"
#include "firstpage.h"
#include "lastpage.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocalizedstring.h>
#include <kiconloader.h>

newDeviceWizard::newDeviceWizard(const QString &configName, QWidget * parent)
    : QWizard(parent)
{
    setObjectName(configName);
    QPixmap wizardLogoPixmap;
    wizardLogoPixmap.load( KGlobal::dirs ()->findResource("data", "kmobiletools/kmobilewizard.png") );
    setPixmap(WatermarkPixmap, wizardLogoPixmap);
    setPixmap(LogoPixmap, KIconLoader::global()->loadIcon("kmobiletools", K3Icon::FirstGroup, K3Icon::SizeHuge ) );
    setWindowTitle(i18nc("new device wizard window title", "KMobileTools New Device Wizard"));
    QWizardPage *curpage=new FirstPage(this);
    addPage( curpage );
    curpage=new LastPage(this);
    setPage( 0xFFFF, curpage);
}

#include "newdevicewizard.moc"
