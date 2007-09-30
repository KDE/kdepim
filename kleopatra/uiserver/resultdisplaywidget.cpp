/* -*- mode: c++; c-basic-offset:4 -*-
    ./resultdisplaywidget.cpp

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

#include "resultdisplaywidget.h"

#include "certificateinfowidgetimpl.h"
#include "utils/formatting.h"

#include <QPointer>

using namespace Kleo;

struct ResultDisplayWidget::Private
{
    Private( ResultDisplayWidget *w ) : q( w )
    {
    }

    ~Private() {}

    ResultDisplayWidget *q;
    QHash<QString, GpgME::Key> keyMap;
    static QHash<QString, QPointer<CertificateInfoWidgetImpl> > dialogMap;
};

QHash<QString, QPointer<CertificateInfoWidgetImpl> > ResultDisplayWidget::Private::dialogMap;

ResultDisplayWidget::ResultDisplayWidget(QWidget * parent) :
    QFrame( parent ),
    d( new Private( this ) )
{
    setObjectName( metaObject()->className() );
}

ResultDisplayWidget::~ResultDisplayWidget()
{
}

QString ResultDisplayWidget::renderKey(const GpgME::Key & key)
{
    if ( key.isNull() )
        return i18n( "Unknown key" );
    const QString keyId = QLatin1String( key.keyID() );
    d->keyMap.insert( keyId, key );
    return QString::fromLatin1( "<a href=\"key:%1\">%2</a>" ).arg( keyId ).arg( Formatting::prettyName( key ) );
}

void Kleo::ResultDisplayWidget::setColor( const QColor &color )
{
    QString css = "QFrame#" + objectName();
    css += " { border:4px solid " + color.name() + "; border-radius:2px; ";
    /*
    css += "background-color: qlineargradient( x1: 0, y1: 0, x2: 0, y2: 1,";
    QColor c = color;
    c.setHsv( c.hue(), 16, c.value() );
    css += "stop: 0.0 " + c.name() + ", ";
    c.setHsv( c.hue(), 27, c.value() );
    css += "stop: 0.4 " + c.name() + ", ";
    c.setHsv( c.hue(), 40, c.value() );
    css += "stop: 0.5 " + c.name() + ", ";
    c.setHsv( c.hue(), 16, c.value() );
    css += "stop: 1.0 " + c.name() + ");";
    */
    css += "}";
    setStyleSheet( css );
}

void ResultDisplayWidget::keyLinkActivated(const QString & link)
{
    if ( !link.startsWith( "key:" ) )
        return;
    const QString keyId = link.mid( 4 );
    if ( d->keyMap.contains( keyId ) ) {
        QPointer<CertificateInfoWidgetImpl> dlg;
        if ( d->dialogMap.contains( keyId ) && d->dialogMap.value( keyId ) ) {
            dlg = d->dialogMap.value( keyId );
            dlg->show();
            dlg->raise();
        } else {
            dlg = new CertificateInfoWidgetImpl( d->keyMap.value( keyId ), false, 0 );
            dlg->setAttribute( Qt::WA_DeleteOnClose );
            dlg->show();
            d->dialogMap.insert( keyId, dlg );
        }
    }
}

#include "resultdisplaywidget.moc"
