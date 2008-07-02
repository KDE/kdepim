/* -*- mode: c++; c-basic-offset:4 -*-
    utils/validation.cpp

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

#include <config-kleopatra.h>

#include "validation.h"

#include <utils/multivalidator.h>

#include <KDebug>

#include <QRegExp>
#include <QUrl>

using namespace Kleo;

static const char email_rx[] = "[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*@(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?";
// these are modeled after gnupg/g10/keygen.c:ask_user_id:
static const char name_rx[] = "[^0-9<>][^<>@]{4,}";
static const char comment_rx[] = "[^()]*";

namespace {

    class EMailValidator : public QRegExpValidator {
    public:
        explicit EMailValidator( QObject * parent=0 ) : QRegExpValidator( QRegExp( email_rx ), parent ) {}

        /* reimp */ void fixup( QString & ) const {}

        /* reimp */ State validate( QString & str, int & pos ) const {
            const int atIdx = str.lastIndexOf( '@' );
            if ( atIdx < 0 )
                return QRegExpValidator::validate( str, pos );

            QString domain = str.mid( atIdx + 1 ).toLower();
            const bool domainEndsWithDot = domain.endsWith( '.' );
            if ( domainEndsWithDot )
                domain.chop( 1 );
            const QByteArray domainEncoded = QUrl::toAce( domain );
            const QString domainRestored = QUrl::fromAce( domainEncoded );
            if ( domain != domainRestored )
                return Invalid;
            QString encoded = str.left( atIdx ) + '@' + QString::fromLatin1( domainEncoded );
            if ( domainEndsWithDot )
                encoded += '.';
            int dummyPosition = 0;
            return QRegExpValidator::validate( encoded, dummyPosition );
        }

    };

}

QValidator * Validation::email( QObject * parent ) {
    return new EMailValidator( parent );
}

QValidator * Validation::email( const QRegExp & addRX, QObject * parent ) {
    return new MultiValidator( email(), new QRegExpValidator( addRX, 0 ), parent );
}

QValidator * Validation::pgpName( QObject * parent ) {
    return new QRegExpValidator( QRegExp( name_rx ), parent );
}

QValidator * Validation::pgpName( const QRegExp & addRX, QObject * parent ) {
    return new MultiValidator( pgpName(), new QRegExpValidator( addRX, 0 ), parent );
}

QValidator * Validation::pgpComment( QObject * parent ) {
    return new QRegExpValidator( QRegExp( comment_rx ), parent );
}

QValidator * Validation::pgpComment( const QRegExp & addRX, QObject * parent ) {
    return new MultiValidator( pgpComment(), new QRegExpValidator( addRX, 0 ), parent );
}

