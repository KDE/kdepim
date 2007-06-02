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
#include "at_devicesfoundpage.h"
#include "at_scanprogresspage.h"
#include <QListWidget>
#include <libkmobiletools/enginedata.h>
#include <kdebug.h>

AT_DevicesFoundPage::AT_DevicesFoundPage(QWidget *parent)
 : DevicesFoundPage(parent)
{
}


AT_DevicesFoundPage::~AT_DevicesFoundPage()
{
}

#include "at_devicesfoundpage.moc"



/*!
    \fn AT_DevicesFoundPage::cleanupPage()
 */
void AT_DevicesFoundPage::cleanupPage()
{
    phonesListWidget()->clear();
}


/*!
    \fn AT_DevicesFoundPage::initializePage()
 */
void AT_DevicesFoundPage::initializePage()
{
    kDebug() << "AT_DevicesFoundPage::initializePage()\n";
    AT_ScanProgressPage *scanpage=(AT_ScanProgressPage *) wizard()->page(wizard()->property("scanprogress_id").toInt() );
    kDebug() << "DevicesFound count: " << scanpage->foundDevices().count() << endl;
    QListIterator<KMobileTools::EngineData*> it(scanpage->foundDevices());
    KMobileTools::EngineData* curitem;
    while(it.hasNext()) {
        curitem=it.next();
        new QListWidgetItem( curitem->property("devicePath").toString(), phonesListWidget() );
    }
}
