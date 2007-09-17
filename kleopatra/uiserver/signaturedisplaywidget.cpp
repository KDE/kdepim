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

#include <QTextEdit>
#include <QHBoxLayout>

#include <gpgme++/verificationresult.h>
#include <gpgme++/key.h>

#include "utils/formatting.h"

using namespace Kleo;
using namespace GpgME;

struct SignatureDisplayWidget::Private {

    Private( SignatureDisplayWidget * w )
    :q( w )
    {
        QHBoxLayout *box = new QHBoxLayout( w );
        textEdit = new QTextEdit(w);
        box->addWidget( textEdit );
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
    
    void reload()
    {
        // fill html text from Signature
        const QString color = colorFromSummary( signature.summary() ).name();
        QString html = QString("<div style=\"background-color:%1\">").arg( color );
        const QString signer = key.isNull() ? "unknown signer" : Formatting::prettyName( key );
        html += QString("<p>Signature by %1</p>").arg( signer );
        html += "</div>";
        
        textEdit->setHtml( html );
    }
    GpgME::Signature signature;
    GpgME::Key key;
    SignatureDisplayWidget *q;
    QTextEdit * textEdit;
};

SignatureDisplayWidget::SignatureDisplayWidget( QWidget* parent )
:QWidget( parent ), d( new Private( this ) )
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


