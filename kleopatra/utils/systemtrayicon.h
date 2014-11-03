/* -*- mode: c++; c-basic-offset:4 -*-
    systemtrayicon.h

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

#ifndef __KLEOPATRA_UTILS_SYSTEMTRAYICON_H__
#define __KLEOPATRA_UTILS_SYSTEMTRAYICON_H__

#include <QSystemTrayIcon>

#ifndef QT_NO_SYSTEMTRAYICON

#include <utils/pimpl_ptr.h>

namespace Kleo
{

class SystemTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    explicit SystemTrayIcon(QObject *parent = 0);
    explicit SystemTrayIcon(const QIcon &icon, QObject *parent = 0);
    ~SystemTrayIcon();

    void setMainWindow(QWidget *w);
    QWidget *mainWindow() const;

    void setAttentionWindow(QWidget *w);
    QWidget *attentionWindow() const;

    QIcon attentionIcon() const;
    QIcon normalIcon() const;
    bool attentionWanted() const;

public Q_SLOTS:
    void setAttentionIcon(const QIcon &icon);
    void setNormalIcon(const QIcon &icon);
    void setAttentionWanted(bool);

protected Q_SLOTS:
    virtual void slotEnableDisableActions() = 0;

private:
    virtual void doMainWindowSet(QWidget *);
    virtual void doMainWindowClosed(QWidget *);
    virtual void doAttentionWindowClosed(QWidget *);
    virtual void doActivated() = 0;

private:
    /* reimp */ bool eventFilter(QObject *, QEvent *);

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT(d, void slotAttentionAnimationTimerTimout())
    Q_PRIVATE_SLOT(d, void slotActivated(QSystemTrayIcon::ActivationReason))
};

} // namespace Kleo

#endif // QT_NO_SYSTEMTRAYICON

#endif /* __KLEOPATRA_UTILS_SYSTEMTRAYICON_H__ */
