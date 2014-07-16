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

#include <cassert>

using namespace Kleo;

static const QString email_rx = QLatin1String("[a-zA-Z0-9!#$%&'*+/=?^_`{|}~-]+(?:\\.[a-zA-Z0-9!#$%&'*+/=?^_`{|}~-]+)*@(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]*[a-zA-Z0-9])?\\.)+[a-zA-Z0-9](?:[a-zA-Z0-9-]*[a-zA-Z0-9])?");
// these are modeled after gnupg/g10/keygen.c:ask_user_id:
static const QString name_rx = QLatin1String("[^0-9<>][^<>@]{4,}");
static const QString comment_rx = QLatin1String("[^()]*");

namespace {

    class EMailValidator : public QValidator {
        QRegExp rx;
    public:
        explicit EMailValidator( QObject * parent=0 ) : QValidator( parent ), rx( QRegExp( email_rx ) ) {}

        /* reimp */ void fixup( QString & ) const {}

        /* reimp */ State validate( QString & str, int & pos ) const {
            const int atIdx = str.lastIndexOf( QLatin1Char('@') );
            if ( atIdx < 0 || str.endsWith( QLatin1Char('@') ) )
                return regexValidate( str, pos );

            // toAce/fromAce doesn't like intermediate domain names,
            // so we fix them up with something innocuous to help it
            // along, and which we strip again afterwards

            QString domain = str.mid( atIdx + 1 ).toLower();
            const int dotIndex = domain.lastIndexOf( QLatin1Char('.') );
            const bool needsOrgAdded = domain.endsWith( QLatin1Char('.') );
            // during typing, the domain might end with '-', which is okay
            // yeah, foo.s also disrupts fromAce, during typing this is okay
            const bool needsDotOrgAdded = !needsOrgAdded && ( dotIndex < 0 || dotIndex == domain.size() - 2 || domain.endsWith( QLatin1Char('-') ) );
            if ( needsOrgAdded )
                domain += QLatin1String("org");
            if ( needsDotOrgAdded )
                domain += QLatin1String("tmp.org");
            const QByteArray domainEncoded = QUrl::toAce( domain );
            const QString domainRestored = QUrl::fromAce( domainEncoded );
            QString encoded = str.left( atIdx ) + QLatin1Char('@') + QString::fromLatin1( domainEncoded );
            if ( needsDotOrgAdded ) {
                assert( encoded.endsWith( QLatin1String( "tmp.org" ) ) );
                encoded.chop( 7 );
            }
            if ( needsOrgAdded ) {
                assert( encoded.endsWith( QLatin1String( ".org" ) ) );
                encoded.chop( 3 ); // '.' was part of domain before
            }
            kDebug() << "\n str           :" << str
                     << "\n domain        :" << domain
                     << "\n domainEncoded :" << domainEncoded
                     << "\n domainRestored:" << domainRestored
                     << "\n encoded       :" << encoded ;
            if ( domain != domainRestored )
                return Invalid;

            // there's no difference between 'encoded' and 'str' at
            // least up to and including 'atIdx', and we need the
            // position for the fixed Intermediate state in
            // regexValidate (e.g. adding a . after marc in
            // marc@kdab.com, intending to eventually arrive at
            // marc.mutz@kdab.com)
            int adjustedPos = pos <= atIdx ? pos : encoded.size() ;
            return regexValidate( encoded, adjustedPos );
        }

    private:
        State regexValidate( QString & input, int & pos ) const {
            // fixed version of QRegExpValidator::validate():
            if (rx.exactMatch(input)) {
                return Acceptable;
            } else {
                if (const_cast<QRegExp &>(rx).matchedLength() >= /*input.size()*/pos) {
                    return Intermediate;
                } else {
                    pos = input.size();
                    return Invalid;
                }
            }
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

