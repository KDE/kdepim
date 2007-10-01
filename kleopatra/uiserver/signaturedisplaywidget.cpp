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
        box->setContentsMargins( 0, 0, 0, 0 );
        summaryLabel = new QLabel( w );
        box->addWidget( summaryLabel );
        signerLabel = new QLabel( w );
        QObject::connect( signerLabel, SIGNAL(linkActivated(QString)), w, SLOT(keyLinkActivated(QString)) );
        box->addWidget( signerLabel );
    }
    ~Private()
    {
    }

    static QColor colorFromSummary( const GpgME::Signature::Summary summary )
    {
        // FIXME make colors configurable
        QColor c = Qt::yellow;
        if ( summary & GpgME::Signature::Green )
            c = Qt::green;
        else if ( summary & GpgME::Signature::Red )
            c = Qt::red;
        return c;
    }

    static QString labelForSummary( const GpgME::Signature::Summary summary )
    {
        QString c = i18n( "Unknown signature state" );
        if ( summary & GpgME::Signature::Green )
            c = i18n( "Valid signature" );
        else if ( summary & GpgME::Signature::Red )
            c = i18n( "Signature invalid" );
        return c;
    }

    static QString iconForSummary( const GpgME::Signature::Summary summary )
    {
        QString c = KIconLoader::global()->iconPath( "dialog-warning", K3Icon::Small );
        if ( summary & GpgME::Signature::Green )
            c = KIconLoader::global()->iconPath( "dialog-ok", K3Icon::Small );
        else if ( summary & GpgME::Signature::Red )
            c = KIconLoader::global()->iconPath( "dialog-error", K3Icon::Small );
        return c;
    }

    void reload()
    {
        q->setColor( colorFromSummary( signature.summary() ) );
        QString l = QString::fromLatin1( "<qt><b><img src=\"%1\"/> " ).arg( iconForSummary( signature.summary() ) );
        l += labelForSummary( signature.summary() );
        l += QLatin1String( "</b></qt>" );
        summaryLabel->setText( l );
        signerLabel->setText( i18n("<p>Signature by %1</p>", q->renderKey( key ) ) );
    }

    GpgME::Signature signature;
    GpgME::Key key;
    SignatureDisplayWidget *q;
    QLabel * summaryLabel;
    QLabel * signerLabel;
};

SignatureDisplayWidget::SignatureDisplayWidget( QWidget* parent )
: ResultDisplayWidget( parent ), d( new Private( this ) )
{
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
