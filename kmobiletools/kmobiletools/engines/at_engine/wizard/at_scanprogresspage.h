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
#ifndef AT_SCANPROGRESSPAGE_H
#define AT_SCANPROGRESSPAGE_H

#include <libkmobiletoolsengineui/scanprogressPage.h>
#include <QList>

class FindDeviceDataJob;
class AT_Engine;
class ATDevicesConfig;
namespace KMobileTools { class EngineData; }
class AT_ScanProgressPage : public ScanProgressPage
{
Q_OBJECT
public:
    AT_ScanProgressPage(QWidget *parent = 0);

    ~AT_ScanProgressPage();
    bool isComplete() const;
    void cleanupPage();
    void initializePage();
    void startScan();
    QList<KMobileTools::EngineData*> foundDevices() { return l_devices ; }

public slots:
    void deviceProbed(FindDeviceDataJob*);
private:
    AT_Engine *engine;
    ATDevicesConfig *cfg;
    int totaljobs;
    int donejobs;
    QList<KMobileTools::EngineData*> l_devices;
};

#endif
