/***************************************************************************
 *   Copyright 2009 by Sebastian Kügler <sebas@kde.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef KPAPPLET_H
#define KPAPPLET_H

//Plasma
#include <Plasma/PopupApplet>
#include <Plasma/ToolTipManager>



class QGraphicsProxyWidget;

namespace KP
{
    class KPDialog;
}

//desktop view
namespace Plasma
{
    class IconWidget;
    class ToolTipContent;
}

class KPApplet : public Plasma::PopupApplet
{
    Q_OBJECT

    public:
        KPApplet(QObject *parent, const QVariantList &args);
        ~KPApplet();
        void init();
        QGraphicsWidget *graphicsWidget();
        void updateToolTip(const int emails);

    protected:
        void popupEvent(bool show);

    private:
        ///the icon used when the applet is in the taskbar
        Plasma::IconWidget *m_icon;

        ///The dialog displaying matches
        KP::KPDialog * m_dialog;

        Plasma::ToolTipContent m_toolTip;

};

#endif
