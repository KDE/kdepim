/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/decryptverifyresultwidget.cpp

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

#include "decryptverifyresultwidget.h"

#include "signaturedisplaywidget.h"

#include <models/keycache.h>
#include <models/predicates.h>

#include <gpgme++/decryptionresult.h>
#include <gpgme++/verificationresult.h>
#include <gpgme++/error.h>
#include <gpgme++/key.h>

#include <KLocale>
#include <KIconLoader>

#include <QLayout>
#include <QLabel>
#include <QColor>
#include <QString>
#include <QTextDocument> // for Qt::escape

using namespace Kleo;
using namespace GpgME;
using namespace boost;

static
// dup of same function in decryptverifycommand.cpp
const GpgME::Key & keyForSignature( const Signature & sig, const std::vector<Key> & keys ) {
    if ( const char * const fpr = sig.fingerprint() ) {
        const std::vector<Key>::const_iterator it
            = std::lower_bound( keys.begin(), keys.end(), fpr, _detail::ByFingerprint<std::less>() );
        if ( it != keys.end() && _detail::ByFingerprint<std::equal_to>()( *it, fpr ) )
            return *it;
    }
    static const Key null;
    return null;
}

static QString image( const char * img ) {
    // ### escape?
    return "<img src=\"" + KIconLoader::global()->iconPath( img, KIconLoader::Small ) + "\"/>";
}

static QColor color( const DecryptionResult & dr, const VerificationResult & vr ) {
    if ( !dr.isNull() && dr.error() )
        return Qt::red;
    if ( !vr.isNull() && vr.error() )
        return Qt::red;
    return Qt::gray;
}

DecryptVerifyResultWidget::DecryptVerifyResultWidget( QWidget * parent )
    : ResultDisplayWidget( parent ),
      m_box( new QVBoxLayout( resultWidget() ) )
{
    m_box->setMargin( 0 );
}

DecryptVerifyResultWidget::~DecryptVerifyResultWidget() {}

void DecryptVerifyResultWidget::setResult( const DecryptionResult & decryptionResult, const VerificationResult & verificationResult ) {

    while ( QLayoutItem * child = m_box->takeAt(0) )
        delete child;

    const QString decryptionResultString = formatDecryptionResult( decryptionResult, KeyCache::instance()->findRecipients( decryptionResult ) );

    m_box->addWidget( new QLabel( decryptionResult.error() || decryptionResult.error().isCanceled()
                                  ? "<qt>" + decryptionResultString + "</qt>"
                                  : "<qt>" + decryptionResultString + "<br/>" + formatVerificationResult( verificationResult ) + "</qt>", resultWidget() ) );
    setColor( color( decryptionResult, verificationResult ) );
        
    const std::vector<Signature> sigs = verificationResult.signatures();
    const std::vector<Key> signers = KeyCache::instance()->findSigners( verificationResult );
    Q_FOREACH ( const Signature & sig, sigs ) {
        SignatureDisplayWidget * w = new SignatureDisplayWidget( resultWidget() );
        w->setSignature( sig, keyForSignature( sig, signers ) );
        m_box->addWidget( w );
    }
    m_box->addStretch( 1 );

    showResultWidget();
}

QString DecryptVerifyResultWidget::formatDecryptionResult( const DecryptionResult & res, const std::vector<Key> & recipients ) {

    if ( res.isNull() )
        return QString();

    const Error err = res.error();

    QString html;

    // Icon:
    html += image( err ? "dialog-error" : "dialog-ok" );

    // Summary:
    html += "<b>";
    if ( err.isCanceled() )
        html += i18n("Decryption canceled.");
    else if ( err )
        html += i18n( "Decryption failed: %1.", Qt::escape( QString::fromLocal8Bit( err.asString() ) ) );
    else
        html += i18n("Decryption succeeded." );
    html += "</b>";

    if ( err.isCanceled() || err || !res.numRecipients() )
        return html;

    // Details (if any):
    html += "<br/>";

    if ( recipients.empty() )
        return html + "<i>" + i18np( "One unknown recipient.", "%n unknown recipients.", res.numRecipients() ) + "</i>";

    html += i18np( "Recipients:", "Recipients:", res.numRecipients() );
    if ( res.numRecipients() == 1 )
        return html + renderKey( recipients.front() );

    html += "<ul>";
    Q_FOREACH( const Key & key, recipients )
        html += "<li>" + renderKey( key ) + "</li>";
    if ( recipients.size() < res.numRecipients() )
        html += "<li><i>" + i18np( "One unknown recipient", "%n unknown recipients",
                                   res.numRecipients() - recipients.size() ) + "</i></li>";

    return html + "</ul>";
}

QString DecryptVerifyResultWidget::formatVerificationResult( const VerificationResult & res ) const {
    if ( res.isNull() )
        return QString();

    const Error err = res.error();

    QString html;

    // Icon:
    html += image( err ? "dialog-error" : "dialog-ok" );

    // Summary:
    html += "<b>";
    if ( err.isCanceled() )
        html += i18n("Verification canceled.");
    else if ( err )
        html += i18n( "Verification failed: %1.", Qt::escape( QString::fromLocal8Bit( err.asString() ) ) );
    else
        html += i18n("Verification succeeded.");
    html += "</b>";

    return html;
}



#include "moc_decryptverifyresultwidget.cpp"
