/* -*- mode: c++; c-basic-offset:4 -*-
    core/command.h

    This file is part of KleopatraClient, the Kleopatra interface library
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    KleopatraClient is free software; you can redistribute it and/or modify
    it under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    KleopatraClient is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __LIBKLEOPATRACLIENT_CORE_COMMAND_H__
#define __LIBKLEOPATRACLIENT_CORE_COMMAND_H__

#include <libkleopatraclient/kleopatraclient_export.h>

#include <QtCore/QObject>
#include <QtGui/QWidget> // only for WId, doesn't prevent linking against QtCore-only

class QString;
class QByteArray;
class QVariant;

namespace KleopatraClient {

    class KLEOPATRACLIENTCORE_EXPORT Command : public QObject {
        Q_OBJECT
    public:
        explicit Command( QObject * parent=0 );
        ~Command();

        void setParentWId( WId wid );
        WId parentWId() const;

        bool waitForFinished();
        bool waitForFinished( unsigned long ms );

        bool error() const;
        QString errorString() const;

        qint64 serverPid() const;

    public Q_SLOTS:
        void start();
        void cancel();

    Q_SIGNALS:
        void started();
        void finished();

    protected:
        void setOptionValue( const char * name, const QVariant & value );
        QVariant optionValue( const char * name ) const;

        void setOption( const char * name );
        void unsetOption( const char * name );
        bool isOptionSet( const char * name ) const;

        QByteArray receivedData() const;

        void setCommand( const char * command );
        QByteArray command() const;

    protected:
        class Private;
        Private * d;
        Command( Private * p, QObject * parent );
    };

}

#endif /* __LIBKLEOPATRACLIENT_CORE_COMMAND_H__ */
