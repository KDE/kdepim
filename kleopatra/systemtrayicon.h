/* -*- mode: c++; c-basic-offset:4 -*-
    systemtrayicon.h

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

#ifndef __KLEOPATRA_SYSTEMTRAYICON_H__
#define __KLEOPATRA_SYSTEMTRAYICON_H__

#include <QSystemTrayIcon>

#include <utils/pimpl_ptr.h>

class SystemTrayIcon : public QSystemTrayIcon {
    Q_OBJECT
public:
    explicit SystemTrayIcon( QObject * parent=0 );
    ~SystemTrayIcon();

    void setMainWindow( QWidget * w );
    QWidget * mainWindow() const;

public Q_SLOTS:
    void openOrRaiseMainWindow();

private:
    virtual QWidget * doCreateMainWindow() const = 0;
    /* reimp */ bool eventFilter( QObject *, QEvent * );

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT( d, void slotAbout() )
    Q_PRIVATE_SLOT( d, void slotActivated( QSystemTrayIcon::ActivationReason ) )
    Q_PRIVATE_SLOT( d, void slotEnableDisableActions() )
    Q_PRIVATE_SLOT( d, void slotEncryptClipboard() )
    Q_PRIVATE_SLOT( d, void slotOpenPGPSignClipboard() )
    Q_PRIVATE_SLOT( d, void slotSMIMESignClipboard() )
    Q_PRIVATE_SLOT( d, void slotDecryptVerifyClipboard() )
};

template <typename T_Widget>
class SystemTrayIconFor : public SystemTrayIcon {
public:
    explicit SystemTrayIconFor( QObject * parent=0 ) : SystemTrayIcon( parent ) {}

    T_Widget * mainWindow() const { return static_cast<T_Widget*>( SystemTrayIcon::mainWindow() ); }

private:
    /* reimp */ QWidget * doCreateMainWindow() const { return new T_Widget; }
};

#endif /* __KLEOPATRA_SYSTEMTRAYICON_H__ */
