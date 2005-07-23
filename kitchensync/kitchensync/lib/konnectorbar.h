/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KSYNC_KONNECTOR_STATUS_BAR_H
#define KSYNC_KONNECTOR_STATUS_BAR_H

#include <qhbox.h>
#include <qlabel.h>
#include <qpixmap.h>

namespace KSync {
    typedef QLabel KonnectorLabel;
    class KonnectorState : public QLabel {
        Q_OBJECT
    public:
        KonnectorState( QWidget* wid );
        ~KonnectorState();
        void setState( bool );
        bool state()const;

    signals:
        void clicked( bool );
    protected:
        void mousePressEvent( QMouseEvent* );

    private:
        int m_state;
        QPixmap m_pix[2];

    };

    /**
     * The konnector bar is meant to represent
     * the connection state of a Konnector and
     * allow toggling connection/disconnection of a konnector
     * Either it is connected or not
     * @version 0.1
     * @author Holger 'zecke' Freyther <freyther@kde.org>
     */
    class KonnectorBar : public QHBox {
        Q_OBJECT
    public:
        enum State { Connected, Disconnected };

        /**
         * The Constructor to construct a KonnectorBox
         * @param parent The parent widget
         * @short Constructor
         */
        KonnectorBar(QWidget* parent );

        /**
         * d'tor
         */
        ~KonnectorBar();

        /**
         * Set the name of the Konnector. The string
         * will be displayed
         * @param name The name of the Konnector
         */
        void setName( const QString& name );

        /**
         * Return the name of the current Konnector
         * @return the name of the konnector
         */
        QString name()const;

        /**
         * Set the state of the connection
         * @param b The state true if connected, false if not connected
         */
        void setState( bool b);

        /**
         * @return the state
         */
        bool state()const;

        /**
         * @return if is connected
         */
        bool isOn()const;
    signals:
        /**
         * if the KonnectarBar got toggled
         * @param b the state of the connection
         */
        void toggled(bool b);

    private:
        KonnectorLabel* m_lbl;
        KonnectorState* m_state;
    };
}


#endif
