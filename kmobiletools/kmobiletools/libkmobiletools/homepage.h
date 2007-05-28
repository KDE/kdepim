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
#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <libkmobiletools/kmobiletools_export.h>

#include <khtml_part.h>

namespace KMobileTools {
class Engine;
}

/**
@author Marco Gulino
*/
class KUrl;
class homepagePartPrivate;

namespace KMobileTools {
class KMOBILETOOLS_EXPORT homepagePart : public KHTMLPart
{
Q_OBJECT
public:
    explicit homepagePart(QWidget *parent = 0, const char *name = 0);

    ~homepagePart();
    static const QString htmlIcon(const QString &iconName, int groupOrSize);
    int currentInfoPage();
private:
    homepagePartPrivate *d;

protected:
    void guiActivateEvent  ( KParts::GUIActivateEvent *event);
    void partActivateEvent( KParts::PartActivateEvent *event );
    void debugPage(const QString &htmldata);
public slots:
    void refreshData(const QString &data);
    void printInfoPage(int i, KMobileTools::Engine *engine);
    void printInfoPage(int i, const QString &mobileName, KMobileTools::Engine *engine);
    void printIndexPage();

protected slots:
    void openUrlRequest(const KUrl &url);
    void slotContextMenu(const QString& urlString, const QPoint& point );

signals:
    void switchDevice(const QString &);
    void deviceCMD(const KUrl &);
    void loadDevice(const QString &);
    void unloadDevice(const QString &);
    void configCmd(const QString &);
    void infopage(int);
    void setStatusBarText(const QString &);

};
}
#endif
