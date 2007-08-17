/* -*- mode: c++; c-basic-offset:4 -*-
    utils/formatting.h

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

#ifndef __KLEOPATRA_UTILS_FORMATTING_H__
#define __KLEOPATRA_UTILS_FORMATTING_H__

#include <gpgme++/key.h>

class QString;
class QDate;

namespace Kleo {
namespace Formatting {

    QString prettyName( int proto, const char * id, const char * name, const char * comment );
    QString prettyName( const GpgME::Key & key );
    QString prettyName( const GpgME::UserID & uid );
    QString prettyName( const GpgME::UserID::Signature & sig );


    QString prettyEMail( const char * email, const char * id );
    QString prettyEMail( const GpgME::Key & key );
    QString prettyEMail( const GpgME::UserID & uid );
    QString prettyEMail( const GpgME::UserID::Signature & sig );


    QString toolTip( const GpgME::Key & key );


    QString expirationDateString( const GpgME::Key & key );
    QString expirationDateString( const GpgME::Subkey & subkey );
    QString expirationDateString( const GpgME::UserID::Signature & sig );
    QDate expirationDate( const GpgME::Key & key );
    QDate expirationDate( const GpgME::Subkey & subkey );
    QDate expirationDate( const GpgME::UserID::Signature & sig );

    QString creationDateString( const GpgME::Key & key );
    QString creationDateString( const GpgME::Subkey & subkey );
    QString creationDateString( const GpgME::UserID::Signature & sig );
    QDate creationDate( const GpgME::Key & key );
    QDate creationDate( const GpgME::Subkey & subkey );
    QDate creationDate( const GpgME::UserID::Signature & sig );


    QString type( const GpgME::Key & key );
    QString type( const GpgME::Subkey & subkey );


    QString validityShort( const GpgME::Subkey & subkey );
    QString validityShort( const GpgME::UserID & uid );
    QString validityShort( const GpgME::UserID::Signature & sig );

}
}

#endif /* __KLEOPATRA_UTILS_FORMATTING_H__ */
