/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/resultpage_p.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#ifndef __KLEOPATRA_CRYPTO_GUI_RESULTPAGE_P_H__
#define __KLEOPATRA_CRYPTO_GUI_RESULTPAGE_P_H__

#include <QWidget>

#include <boost/shared_ptr.hpp>

class QLabel;

namespace Kleo {
namespace Crypto {

class Task;

namespace Gui {

    class ResultItemWidget : public QWidget {
        Q_OBJECT
    public:
        ResultItemWidget( const boost::shared_ptr<const Task::Result> &result, const QString & taskLabel, QWidget * parent=0, Qt::WindowFlags flags=0 );
        ~ResultItemWidget();

        bool detailsVisible() const;
        bool hasErrorResult() const;

    public Q_SLOTS:
        void showDetails( bool show = true );

    Q_SIGNALS:
        void linkActivated( const QString & link );

    private Q_SLOTS:
        void slotLinkActivated( const QString& );

    private:
        void updateShowDetailsLabel();

    private:
        const boost::shared_ptr<const Task::Result> m_result;
        QLabel * m_detailsLabel;
        QLabel * m_showDetailsLabel;
    };

}
}
}

#endif // __KLEOPATRA_CRYPTO_GUI_RESULTPAGE_P_H__
