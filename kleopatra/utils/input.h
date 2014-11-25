/* -*- mode: c++; c-basic-offset:4 -*-
    utils/input.h

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

#ifndef __KLEOPATRA_UTILS_INPUT_H__
#define __KLEOPATRA_UTILS_INPUT_H__

#include <kleo-assuan.h> // for assuan_fd_t

#include <boost/shared_ptr.hpp>

class QIODevice;
class QString;
class QStringList;
class QByteArray;
class QFile;
class QDir;

namespace Kleo {

    class Input {
    public:
        virtual ~Input();

        virtual QString label() const = 0;
        virtual void setLabel( const QString & label ) = 0;
        virtual boost::shared_ptr<QIODevice> ioDevice() const = 0;
        virtual unsigned int classification() const = 0;
        virtual unsigned long long size() const = 0;
        virtual QString errorString() const = 0;
        /** Whether or not the input failed. */
        virtual bool failed() const { return false; }
        void finalize(); // equivalent to ioDevice()->close();

        static boost::shared_ptr<Input> createFromPipeDevice( assuan_fd_t fd, const QString & label );
        static boost::shared_ptr<Input> createFromFile( const QString & filename, bool dummy=false );
        static boost::shared_ptr<Input> createFromFile( const boost::shared_ptr<QFile> & file );
        static boost::shared_ptr<Input> createFromProcessStdOut( const QString & command );
        static boost::shared_ptr<Input> createFromProcessStdOut( const QString & command, const QStringList & args );
        static boost::shared_ptr<Input> createFromProcessStdOut( const QString & command, const QStringList & args, const QDir & workingDirectory );
        static boost::shared_ptr<Input> createFromProcessStdOut( const QString & command, const QByteArray & stdin_ );
        static boost::shared_ptr<Input> createFromProcessStdOut( const QString & command, const QStringList & args, const QByteArray & stdin_ );
        static boost::shared_ptr<Input> createFromProcessStdOut( const QString & command, const QStringList & args, const QDir & workingDirectory, const QByteArray & stdin_ );
#ifndef QT_NO_CLIPBOARD
        static boost::shared_ptr<Input> createFromClipboard();
#endif
    };
}

#endif /* __KLEOPATRA_UTILS_INPUT_H__ */

