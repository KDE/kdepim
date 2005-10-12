/*  This file is part of the KDE kmobile library.
    Copyright (C) 2003 Helge Deller <deller@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include <ksystemtray.h>
//Added by qt3to4:
#include <QPixmap>
#include <QLabel>

class KMainWindow;
class KAction;

class SystemTray : public KSystemTray
{
    Q_OBJECT

public:
    SystemTray(KMainWindow *parent = 0, const char *name = 0);
    virtual ~SystemTray();

protected slots:
    void menuItemActivated(int id);
    void menuItemSelected();

protected:
    void contextMenuAboutToShow( KMenu* menu );

private:
    void setToolTip(const QString &tip = QString::null);

    QPixmap m_appPix;
    QLabel *m_currentLabel;

    KActionCollection *m_actionCollection;
    int m_menuID;
};

#endif // SYSTEMTRAY_H
