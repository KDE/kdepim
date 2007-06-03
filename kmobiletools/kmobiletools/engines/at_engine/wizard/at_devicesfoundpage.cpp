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
#include <libkmobiletools/engineslist.h>
#include <kdebug.h>
#include <QLabel>
#include <QStackedWidget>
#include <klocalizedstring.h>
#include <QVBoxLayout>
#include "at_engine.h"

AT_DevicesFoundPage::AT_DevicesFoundPage(QWidget *parent)
 : DevicesFoundPage(parent)
{
    b_details=new QLabel();
    details()->addWidget(b_details);
    details()->setCurrentWidget(b_details);
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
    registerField("at_lv_device", phonesListWidget() );
    kDebug() << "AT_DevicesFoundPage::initializePage()\n";
    AT_ScanProgressPage *scanpage=(AT_ScanProgressPage *) wizard()->page(wizard()->property("scanprogress_id").toInt() );
    kDebug() << "DevicesFound count: " << scanpage->foundDevices().count() << endl;
    QListIterator<KMobileTools::EngineData*> it(scanpage->foundDevices());
    KMobileTools::EngineData* curitem;
    QListWidgetItem *newlwitem;
    while(it.hasNext()) {
        curitem=it.next();
        newlwitem=new QListWidgetItem( curitem->manufacturerString() + " " + curitem->model() , phonesListWidget() );
        newlwitem->setData(Qt::UserRole+1, curitem->property("devicePath").toString() ); /// @TODO replace with some model/view stuff
        newlwitem->setData(Qt::UserRole+2, curitem->imei() ); /// @TODO as above...
    }
}

void AT_DevicesFoundPage::slotDetails(QListWidgetItem *item) {
    kDebug() << k_funcinfo << endl;
    enginedata=0;
    if(!item) return;
    AT_ScanProgressPage *scanpage=(AT_ScanProgressPage *) wizard()->page(wizard()->property("scanprogress_id").toInt() );

    QListIterator<KMobileTools::EngineData*> it(scanpage->foundDevices());
    KMobileTools::EngineData* curitem; /// @TODO replace with some model/view stuff
    while(it.hasNext()) {
        curitem=it.next();
        if(curitem->imei() == item->data(Qt::UserRole+2).toString() )
            enginedata=curitem;
    }
    showDetails(enginedata);
}


/*!
    \fn AT_DevicesFoundPage::showDetails(KMobileTools::EngineData* engineData)
 */
void AT_DevicesFoundPage::showDetails(KMobileTools::EngineData* engineData)
{
    kDebug() << k_funcinfo << endl;
    kDebug() << "EngineData: " << engineData << endl;
    if(!engineData) {
        b_details->setText(QString() );
        return;
    }
    b_details->setText(i18nc("AT Engine wizard - device details html code",
    "<qt><ul><li>Manufacturer: %1</li>\
    <li>Model: %2</li>\
    <li>IMEI: %3</li>\
    <li>Device: %4</li>",
    engineData->manufacturerString(), engineData->model(), engineData->imei(), engineData->property("devicePath").toString()
    ) );
}

bool AT_DevicesFoundPage::validatePage() {
    if(!enginedata) return false;
    ATDevicesConfig *cfg=(ATDevicesConfig*) DEVCFG(wizard()->objectName() );
    cfg->setMobileimei(enginedata->imei());
    cfg->setRawdevicevendor(enginedata->manufacturerString());
    cfg->setRawdevicename(enginedata->model());
    cfg->writeConfig();
    /// @TODO set slots
}
