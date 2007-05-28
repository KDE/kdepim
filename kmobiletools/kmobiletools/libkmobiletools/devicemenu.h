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
#ifndef DEVICEMENU_H
#define DEVICEMENU_H

#include <kmenu.h>
#include <kurl.h>

/**
@author Marco Gulino
*/
namespace KMobileTools {
class Engine;
class deviceMenu : public KMenu
{
Q_OBJECT
public:
explicit deviceMenu(bool loaded=false, KMobileTools::Engine *engine=NULL, QWidget *parent = 0, const QString &name = QString());

    ~deviceMenu();
private:
    KMobileTools::Engine *p_engine;

public slots:
    void slotLoadDevice();
    void slotSwitchDevice();
    void slotUnloadDevice();
    void slotConfigureDevice();
    void slotGoSMS();
    void slotGoPhonebook();

signals:
    void switchDevice(const QString &);
    void loadDevice(const QString &);
    void unloadDevice(const QString &);
    void configure(const QString &);
    void sendURL(const KUrl&);
};
}
#endif
