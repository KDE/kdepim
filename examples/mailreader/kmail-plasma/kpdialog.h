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

#ifndef KPDIALOG_H
#define KPDIALOG_H

// Akonadi
#include <AkonadiCore/collection.h>
#include <AkonadiCore/item.h>

//Qt
#include <QGraphicsWidget>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QStringList>

// KDE
#include <KLineEdit>
#include <KPushButton>


//own
class KPApplet;

//desktop view
namespace Plasma
{
    class Icon;
    class Dialog;
    class TabBar;
}

namespace Akonadi
{
    class EntityTreeView;
}

namespace MessageList
{
    class Pane;
}

namespace KP
{
  /**
  * @short KMail's message list in a popup applet
  *
  */
  class KPDialog : public QGraphicsWidget
  {
  Q_OBJECT

    public:
        /**
        * Constructor of the dialog
        * @param kpapplet the KPApplet attached to this dialog
        * @param parent the parent of this object
        **/
        KPDialog(KPApplet * kpapplet, QGraphicsWidget *parent = 0);

        virtual ~KPDialog();

        /**
        * Returns the related QWidget.
        **/
        QGraphicsWidget * dialog();

    private Q_SLOTS:
        /**
        * @internal update the color of the label to follow plasma theme
        *
        **/
        void updateColors();

    private :
        /**
        * @internal build the dialog depending where it is
        **/
        void buildDialog();
        void setupPane();
        Plasma::TabBar* m_tabs;
        QGraphicsProxyWidget *m_folderListProxyWidget;
        QGraphicsProxyWidget *m_messageListProxyWidget;
        QWidget *m_folderListWidget;
        QWidget *m_messageListWidget;
        KPushButton * m_button;
        MessageList::Pane *m_messagePane;
        Akonadi::EntityTreeView *m_folderListView;
        KPApplet * m_applet;
  };
}

#endif

