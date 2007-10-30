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

#include "models/predicates.h"

#include <QHash>
#include <QPointer>

#include <QLayout>
#include <QLabel>
#include <QProgressBar>
#include <QStackedLayout>
#include <QFrame>

#include <algorithm>
#include <cassert>

using namespace Kleo;
using namespace GpgME;

static const char ERROR_STYLE_SHEET[] =
    "border:4px solid red; "
    "border-radius:2px;";

class ProgressWidget : public QWidget {
    Q_OBJECT
public:
    explicit ProgressWidget( QWidget * p=0 )
        : QWidget( p ), ui( this ) {}

public Q_SLOTS:
    void setText( const QString & text ) {
        ui.label.setText( text );
    }
    void setProgress( int current, int total ) {
        ui.progress.setRange( 0, total );
        ui.progress.setValue( current );
    }

private:
    struct UI {
        QVBoxLayout vlay;
        QLabel       label;
        QProgressBar progress;

        explicit UI( ProgressWidget * q )
            : vlay( q ),
              label( q ),
              progress( q )
        {
            KDAB_SET_OBJECT_NAME( vlay );
            KDAB_SET_OBJECT_NAME( label );
            KDAB_SET_OBJECT_NAME( progress );

            progress.setRange( 0, 0 ); // knight rider mode

            vlay.addWidget( &label );
            vlay.addWidget( &progress );
        }
    } ui;
};


class ResultDisplayWidget::Private {
    friend class ::Kleo::ResultDisplayWidget;
    ResultDisplayWidget * const q;
public:
    explicit Private( ResultDisplayWidget * qq )
        : q( qq ),
          ui( q )
    {
        connect( &ui.stack, SIGNAL(currentChanged(int)),
                 q, SIGNAL(operationStateChanged()) );
    }

    std::vector<Key> keys;
    static QHash<QString, QPointer<CertificateInfoWidgetImpl> > dialogMap;

    struct UI {
        ProgressWidget  progress;
        QFrame          result;
        QLabel          error;
        QStackedLayout stack;

        explicit UI( ResultDisplayWidget * q )
            : progress( q ),
              result( q ),
              error( q ),
              stack( q )
        {
            KDAB_SET_OBJECT_NAME( progress );
            KDAB_SET_OBJECT_NAME( result );
            KDAB_SET_OBJECT_NAME( error );
            KDAB_SET_OBJECT_NAME( stack );

            stack.setMargin( 0 );

            error.setStyleSheet( ERROR_STYLE_SHEET );

            stack.addWidget( &progress );
            stack.addWidget( &result );
            stack.addWidget( &error );

            stack.setCurrentIndex( 0 );
        }
    } ui;
};

QHash<QString, QPointer<CertificateInfoWidgetImpl> > ResultDisplayWidget::Private::dialogMap;

ResultDisplayWidget::ResultDisplayWidget( QWidget * p )
    : QWidget( p ), d( new Private( this ) )
{
    setObjectName( "Kleo__ResultDisplayWidget" );
}

ResultDisplayWidget::~ResultDisplayWidget() {}

bool ResultDisplayWidget::operationInProgress() const {
    return d->ui.stack.currentWidget() == &d->ui.progress ;
}

QString ResultDisplayWidget::renderKey(const Key & key)
{
    if ( key.isNull() )
        return i18n( "Unknown key" );
    d->keys.push_back( key );
    std::inplace_merge( d->keys.begin(), d->keys.end() - 1, d->keys.end(), _detail::ByFingerprint<std::less>() );
    return QString::fromLatin1( "<a href=\"key:%1\">%2</a>" ).arg( key.primaryFingerprint(), Formatting::prettyName( key ) );
}

static QColor resaturate( const QColor & c, int sat ) {
    int h, s, v;
    c.getHsv( &h, &s, &v );
    return QColor::fromHsv( h, sat, v );
}

// static
QString Kleo::ResultDisplayWidget::styleSheet( const QColor & c ) {
    return
        "border:4px solid " + c.name() + "; border-radius:2px; "
        "background-color: qlineargradient( x1: 0, y1: 0, x2: 0, y2: 1, "
        "stop: 0.0 " + resaturate( c, 16 ).name() + ", "
        "stop: 0.4 " + resaturate( c, 27 ).name() + ", "
        "stop: 0.5 " + resaturate( c, 40 ).name() + ", "
        "stop: 1.0 " + resaturate( c, 16 ).name() + ");"
        ;
}

void Kleo::ResultDisplayWidget::setColor( const QColor &color )
{
    setStyleSheet( '#' + objectName() + '{' + styleSheet( color ) + '}' );
}

void ResultDisplayWidget::keyLinkActivated(const QString & link)
{
    if ( !link.startsWith( "key:" ) )
        return;
    const QString fpr = link.mid( 4 );
    const std::vector<Key>::const_iterator kit
        = qBinaryFind( d->keys.begin(), d->keys.end(), fpr.toStdString(), _detail::ByFingerprint<std::less>() );
    if ( kit == d->keys.end() )
        return;

    QHash<QString, QPointer<CertificateInfoWidgetImpl> >::const_iterator dit = d->dialogMap.find( fpr );
    if ( dit == d->dialogMap.end() || !*dit ) {
        CertificateInfoWidgetImpl * const dlg = new CertificateInfoWidgetImpl( *kit, false, 0 );
        dlg->setAttribute( Qt::WA_DeleteOnClose );
        dit = d->dialogMap.insert( fpr, dlg );
    }
    assert( dit != d->dialogMap.end() );

    if ( (*dit)->isVisible() )
        (*dit)->raise();
    else
        (*dit)->show();
}

void ResultDisplayWidget::setLabel( const QString & label ) {
    d->ui.progress.setText( label );
    d->ui.stack.setCurrentIndex( 0 );
}

void ResultDisplayWidget::setProgress( const QString & what, int current, int total ) {
    d->ui.progress.setText( what );
    d->ui.progress.setProgress( current, total );
}

void ResultDisplayWidget::showResultWidget() {
    d->ui.stack.setCurrentIndex( 1 );
}

void ResultDisplayWidget::setError( const QString & err ) {
    d->ui.error.setText( err );
    d->ui.stack.setCurrentIndex( 2 );
}

QWidget * ResultDisplayWidget::resultWidget() {
    return &d->ui.result;
}

#include "resultdisplaywidget.moc"
#include "moc_resultdisplaywidget.cpp"
