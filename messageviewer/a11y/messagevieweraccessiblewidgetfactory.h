/*
    Copyright 2011  José Millán Soto <fid@gpul.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KDEPIMACCESSIBLEWIDGETFACTORY_H
#define KDEPIMACCESSIBLEWIDGETFACTORY_H

#include <qaccessible.h>
#include <QAccessiblePlugin>

class MessageViewerAccessibleWidgetFactory: public QAccessiblePlugin {
public:
    explicit MessageViewerAccessibleWidgetFactory( QObject *parent = 0 );
    virtual QAccessibleInterface *create( const QString &key, QObject *object );
    virtual QStringList keys() const;
};

#endif
