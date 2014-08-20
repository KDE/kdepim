/* -*- mode: c++; c-basic-offset:4 -*-
    utils/systemtrayicon.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007,2009 Klarälvdalens Datakonsult AB

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

#include "systemtrayicon.h"

#ifndef QT_NO_SYSTEMTRAYICON


#include <QDebug>
#include <QTimer>
#include <QPointer>
#include <QWidget>
#include <QEvent>

#include <cassert>

using namespace Kleo;

static const int ATTENTION_ANIMATION_FRAMES_PER_SEC = 1;

class SystemTrayIcon::Private {
    friend class ::SystemTrayIcon;
    SystemTrayIcon * const q;
public:
    explicit Private( SystemTrayIcon * qq );
    ~Private();

private:
    bool attentionWanted() const {
        return attentionAnimationTimer.isActive();
    }

    void setAttentionWantedImpl( bool on ) {
        if ( on ) {
            attentionAnimationTimer.start();
        } else {
            attentionAnimationTimer.stop();
            attentionIconShown = false;
            q->setIcon( normalIcon );
        }
    }

    void slotActivated( ActivationReason reason ) {
        if ( reason == QSystemTrayIcon::Trigger )
            q->doActivated();
    }

    void slotAttentionAnimationTimerTimout() {
        if ( attentionIconShown ) {
            attentionIconShown = false;
            q->setIcon( normalIcon );
        } else {
            attentionIconShown = true;
            q->setIcon( attentionIcon );
        }
    }

private:
    bool attentionIconShown;

    QIcon normalIcon, attentionIcon;

    QTimer attentionAnimationTimer;

    QPointer<QWidget> mainWindow;
    QPointer<QWidget> attentionWindow;
};

SystemTrayIcon::Private::Private( SystemTrayIcon * qq )
    : q( qq ),
      attentionIconShown( false ),
      attentionAnimationTimer(),
      mainWindow(),
      attentionWindow()
{
    KDAB_SET_OBJECT_NAME( attentionAnimationTimer );

    attentionAnimationTimer.setSingleShot( false );
    attentionAnimationTimer.setInterval( 1000 * ATTENTION_ANIMATION_FRAMES_PER_SEC / 2 );

    connect( q, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), q, SLOT(slotActivated(QSystemTrayIcon::ActivationReason)) );
    connect( &attentionAnimationTimer, SIGNAL(timeout()), q, SLOT(slotAttentionAnimationTimerTimout()) );
}

SystemTrayIcon::Private::~Private() {}

SystemTrayIcon::SystemTrayIcon( QObject * p )
    : QSystemTrayIcon( p ), d( new Private( this ) )
{

}

SystemTrayIcon::SystemTrayIcon( const QIcon & icon, QObject * p )
    : QSystemTrayIcon( icon, p ), d( new Private( this ) )
{
    d->normalIcon = d->attentionIcon = icon;
}

SystemTrayIcon::~SystemTrayIcon() {}

void SystemTrayIcon::setMainWindow( QWidget * mw ) {
    if ( d->mainWindow )
        return;
    d->mainWindow = mw;
    if ( mw )
        mw->installEventFilter( this );
    doMainWindowSet( mw );
    slotEnableDisableActions();
}

QWidget * SystemTrayIcon::mainWindow() const {
    return d->mainWindow;
}

void SystemTrayIcon::setAttentionWindow( QWidget * mw ) {
    if ( d->attentionWindow )
        return;
    d->attentionWindow = mw;
    if ( mw )
        mw->installEventFilter( this );
    slotEnableDisableActions();
}

QWidget * SystemTrayIcon::attentionWindow() const {
    return d->attentionWindow;
}

bool SystemTrayIcon::eventFilter( QObject * o, QEvent * e ) {
    if ( o == d->mainWindow )
        switch ( e->type() ) {
        case QEvent::Close:
            doMainWindowClosed( static_cast<QWidget*>( o ) );
            // fall through:
        case QEvent::Show:
        case QEvent::DeferredDelete:
            QMetaObject::invokeMethod( this, "slotEnableDisableActions", Qt::QueuedConnection );
        default: ;
        }
    else if ( o == d->attentionWindow )
        switch ( e->type() ) {
        case QEvent::Close:
            doAttentionWindowClosed( static_cast<QWidget*>( o ) );
            // fall through:
        case QEvent::Show:
        case QEvent::DeferredDelete:
            QMetaObject::invokeMethod( this, "slotEnableDisableActions", Qt::QueuedConnection );
        default: ;
        }
    return false;
}

void SystemTrayIcon::setAttentionWanted( bool on ) {
    if ( d->attentionWanted() == on )
        return;
    qDebug() << d->attentionWanted() << "->" << on;
    d->setAttentionWantedImpl( on );
}

bool SystemTrayIcon::attentionWanted() const {
    return d->attentionWanted();
}

void SystemTrayIcon::setNormalIcon( const QIcon & icon ) {
    if ( d->normalIcon.cacheKey() == icon.cacheKey() )
        return;
    d->normalIcon = icon;
    if ( !d->attentionWanted() || !d->attentionIconShown )
        setIcon( icon );
}

QIcon SystemTrayIcon::normalIcon() const {
    return d->normalIcon;
}

void SystemTrayIcon::setAttentionIcon( const QIcon & icon ) {
    if ( d->attentionIcon.cacheKey() == icon.cacheKey() )
        return;
    d->attentionIcon = icon;
    if ( d->attentionWanted() && d->attentionIconShown )
        setIcon( icon );
}

QIcon SystemTrayIcon::attentionIcon() const {
    return d->attentionIcon;
}

void SystemTrayIcon::doMainWindowSet( QWidget * ) {}
void SystemTrayIcon::doMainWindowClosed( QWidget * ) {}
void SystemTrayIcon::doAttentionWindowClosed( QWidget * ) {}

#include "moc_systemtrayicon.cpp"

#endif // QT_NO_SYSTEMTRAYICON
