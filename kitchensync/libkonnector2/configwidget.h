/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <freyther@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KSYNC_KONNECTOR_CONFIG_WIDGET_H
#define KSYNC_KONNECTOR_CONFIG_WIDGET_H

#include <qwidget.h>

#include "kapabilities.h"

namespace KSync {
    /**
     * This is a baseclass for a configuration GUI provided
     * by a Konnector
     * It provides methods to get a Kapaibility and to set
     * Kapabilities
     *
     */
    class ConfigWidget : public QWidget {
        Q_OBJECT
    public:
        /** normal QWidget like c'tor + a Kapabilities object */
        ConfigWidget( const Kapabilities&, QWidget* parent,  const char* name );

        /** another c'tor */
        ConfigWidget( QWidget* parent, const char* name );

        /** d'tor */
        virtual ~ConfigWidget();

        /** returns a capability of the widget */
        virtual Kapabilities capabilities() const = 0;

        /** set this widget to show a capability */
        virtual void setCapabilities( const Kapabilities& ) = 0;
    private:
        class ConfigWidgetPrivate;
        ConfigWidgetPrivate* d;

    };
}
#endif
