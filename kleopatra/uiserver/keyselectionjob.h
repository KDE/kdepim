/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/keyselectionjob.h

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

#ifndef __KLEO_KEYSELECTIONJOB_H__
#define __KLEO_KEYSELECTIONJOB_H__

#include <utils/pimpl_ptr.h>

#include <QObject>

#include <vector>

namespace GpgME {
    class Error;
    class Key;
    class KeyListResult;
}

namespace Kleo {

class KeySelectionJob : public QObject
{
    Q_OBJECT
public:
    explicit KeySelectionJob( QObject* parent = 0 );

    void setPatterns( const QStringList& patterns );
    QStringList patterns() const;

    void setSecretKeysOnly( bool secretOnly );
    bool secretKeysOnly() const;

    void setSilent( bool silent );
    bool silent() const;

    void start();

Q_SIGNALS:
    void error( const GpgME::Error& error, const GpgME::KeyListResult& result );
    void result( const std::vector<GpgME::Key>& keys );

private:
    Q_PRIVATE_SLOT( d, void nextKey( const GpgME::Key& key ) )
    Q_PRIVATE_SLOT( d, void keyListingDone( const GpgME::KeyListResult& result ) )
    Q_PRIVATE_SLOT( d, void keySelectionDialogClosed() )

    class Private;
    kdtools::pimpl_ptr<Private> d;
};

}

#endif /*__KLEO_KEYSELECTIONJOB_H__*/
