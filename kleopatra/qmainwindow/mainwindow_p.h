/* -*- mode: c++; c-basic-offset:4 -*-
    mainwindow_p.h

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


#ifndef __KLEOPATRA_MAINWINDOW_P_H__
#define __KLEOPATRA_MAINWINDOW_P_H__

#include <QObject>

#include <vector>

#include <gpgme++/context.h>
#include <gpgme++/error.h>
#include <gpgme++/key.h>

class QStatusBar;

namespace Kleo {
    class KeyListJob;
}

namespace {
class Relay : public QObject {
    Q_OBJECT
public:
    explicit Relay( QObject * p=0 ) : QObject( p ) {}

public Q_SLOTS:
    void nextKey( const GpgME::Key & key ) {
        qDebug( "next key" );
        mKeys.push_back( key );
        // push out keys in chunks of 1..16 keys
        if ( mKeys.size() > qrand() % 16U )
            flushKeys();
    }

    void keyListingDone( const GpgME::KeyListResult& result ) {
        Q_UNUSED( result );
        flushKeys();
    }
 

Q_SIGNALS:
    void nextKeys( const std::vector<GpgME::Key> & keys );

private:
    void flushKeys() {
        emit nextKeys( mKeys );
        mKeys.clear();
    }

private:
    std::vector<GpgME::Key> mKeys;
};

class StatusBarUpdater : public QObject {
    Q_OBJECT
public:
    explicit StatusBarUpdater( QStatusBar* bar, QObject* parent = 0 );

    void registerJob( Kleo::KeyListJob* job );
                                             
private Q_SLOTS:
    void keyListResult( const GpgME::KeyListResult& result );

private:
    int m_pendingJobs;
    QStatusBar* m_bar;
};

}

#endif // __KLEOPATRA_MAINWINDOW_P_H__
