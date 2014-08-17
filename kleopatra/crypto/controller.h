/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/controller.h

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

#ifndef __KLEOPATRA_CRYPTO_CONTROLLER_H__
#define __KLEOPATRA_CRYPTO_CONTROLLER_H__

#include <QObject>

#include <crypto/task.h>

#include <utils/pimpl_ptr.h>
#include <utils/types.h>

#include <boost/shared_ptr.hpp>


namespace Kleo {
namespace Crypto {

    class Controller : public QObject, protected ExecutionContextUser {
        Q_OBJECT
    public:
        explicit Controller( QObject * parent=0 );
        explicit Controller( const boost::shared_ptr<const ExecutionContext> & cmd, QObject * parent=0 );
        ~Controller();

        using ExecutionContextUser::setExecutionContext;

    Q_SIGNALS:
        void progress( int current, int total, const QString & what );

    protected:
        void emitDoneOrError();
        void setLastError( int err, const QString & details );
        void connectTask( const boost::shared_ptr<Task> & task );

        virtual void doTaskDone( const Task* task, const boost::shared_ptr<const Task::Result> & result );

    protected Q_SLOTS:
        void taskDone( const boost::shared_ptr<const Kleo::Crypto::Task::Result> & );

    Q_SIGNALS:

#ifndef Q_MOC_RUN
# ifndef DOXYGEN_SHOULD_SKIP_THIS
    private: // don't tell moc or doxygen, but those signals are in fact private
# endif
#endif
        void error( int err, const QString & details );
        void done();

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
    };

}
}

#endif /* __KLEOPATRA_CRYPTO_CONTROLLER_H__ */
