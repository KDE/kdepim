/* -*- mode: c++; c-basic-offset:4 -*-
    controllers/keylistcontroller.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/
#ifndef __KLEOPATRA_CONTROLLERS_KEYLISTCONTROLLER_H__
#define __KLEOPATRA_CONTROLLERS_KEYLISTCONTROLLER_H__

#include <QObject>

#include <utils/pimpl_ptr.h>

#include <vector>

class QAbstractItemView;

namespace Kleo {

    class AbstractKeyListModel;

    class KeyListController : public QObject {
        Q_OBJECT
    public:
        explicit KeyListController( QObject * parent=0 );
        ~KeyListController();

        void addView( QAbstractItemView * view );
        void removeView( QAbstractItemView * view );
        std::vector<QAbstractItemView*> views() const;

        void setModel( AbstractKeyListModel * model );
        AbstractKeyListModel * model() const;

        

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;

        Q_PRIVATE_SLOT( d, void slotDestroyed(QObject*) )
        Q_PRIVATE_SLOT( d, void slotDoubleClicked(QModelIndex) )
        Q_PRIVATE_SLOT( d, void slotActivated(QModelIndex) )
        Q_PRIVATE_SLOT( d, void slotSelectionChanged(QItemSelection,QItemSelection) )
        Q_PRIVATE_SLOT( d, void slotContextMenu(QPoint) )
        Q_PRIVATE_SLOT( d, void slotCommandFinished() )
    };

}

#endif /* __KLEOPATRA_CONTROLLERS_KEYLISTCONTROLLER_H__ */
