/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/siganturedisplaywidget.cpp

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


#include "signaturedisplaywidget.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QPainter>

#include <kiconloader.h>
#include <klocale.h>

#include <gpgme++/verificationresult.h>
#include <gpgme++/key.h>

#include "certificateinfowidgetimpl.h"
#include "utils/formatting.h"

using namespace Kleo;
using namespace GpgME;

struct SignatureDisplayWidget::Private {

    Private( SignatureDisplayWidget * w )
    :q( w )
    {
        QVBoxLayout *box = new QVBoxLayout( w );
        summaryLabel = new QLabel( w );
        box->addWidget( summaryLabel );
        signerLabel = new QLabel( w );
        QObject::connect( signerLabel, SIGNAL(linkActivated(QString)), w, SLOT(linkActivated(QString)) );
        box->addWidget( signerLabel );
    }
    ~Private()
    {
    }

    static QColor colorFromSummary( const GpgME::Signature::Summary summary )
    {
        // FIXME make colors configurable
        QColor c = Qt::yellow;
        switch ( summary ) {
        case GpgME::Signature::Green:
            c = Qt::green;
            break;
        case GpgME::Signature::Red:
            c = Qt::red;
            break;
        default:
            break;
        }
        return c;
    }

    static QString labelForSummary( const GpgME::Signature::Summary summary )
    {
        switch ( summary ) {
            case GpgME::Signature::Green:
                return i18n( "Valid signature" );
            case GpgME::Signature::Red:
                return i18n( "Signaute invalid" );
            default:
                return i18n( "Unknown signature state" );
        }
        return QString();
    }

    static QString iconForSummary( const GpgME::Signature::Summary summary )
    {
        switch ( summary ) {
            case GpgME::Signature::Green:
                return KIconLoader::global()->iconPath( "dialog-ok", K3Icon::Small );
            case GpgME::Signature::Red:
                return KIconLoader::global()->iconPath( "dialog-error", K3Icon::Small );
            default:
                return KIconLoader::global()->iconPath( "dialog-warning", K3Icon::Small );
        }
        return QString();
    }

    void reload()
    {
        q->setStyleSheet( QString::fromLatin1("QFrame#SignatureDisplayWidget { border:4px solid %1; border-radius:2px; }")
                .arg( colorFromSummary( signature.summary() ).name() ) );
        QString l = QString::fromLatin1( "<qt><b><img src=\"%1\"/> " ).arg( iconForSummary( signature.summary() ) );
        l += labelForSummary( signature.summary() );
        l += QLatin1String( "</b></qt>" );
        summaryLabel->setText( l );

        QString signer;
        if ( key.isNull() )
            signer = i18n( "unknown signer" );
        else
            signer = QString::fromLatin1( "<a href=\"showKey\">%1</a>" ).arg( Formatting::prettyName( key ) );
        signerLabel->setText( i18n("<p>Signature by %1</p>", signer) );
    }

    void linkActivated( const QString &link )
    {
        if ( link == "showKey" ) {
            CertificateInfoWidgetImpl *dlg = new CertificateInfoWidgetImpl( key, false, 0 );
            dlg->setAttribute( Qt::WA_DeleteOnClose );
            dlg->show();
        }
    }

    GpgME::Signature signature;
    GpgME::Key key;
    SignatureDisplayWidget *q;
    QLabel * summaryLabel;
    QLabel * signerLabel;
};

SignatureDisplayWidget::SignatureDisplayWidget( QWidget* parent )
:QFrame( parent ), d( new Private( this ) )
{
    setObjectName( "SignatureDisplayWidget" );
}

SignatureDisplayWidget::~SignatureDisplayWidget()
{
}

void Kleo::SignatureDisplayWidget::setSignature(const GpgME::Signature & sig, const GpgME::Key & signingkey )
{
    d->signature = sig;
    d->key = signingkey;
    d->reload();
}

#include "signaturedisplaywidget.moc"
