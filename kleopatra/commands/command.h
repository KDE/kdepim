/* -*- mode: c++; c-basic-offset:4 -*-
    commands/command.h

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

#ifndef __KLEOPATRA_COMMANDS_COMMAND_H__
#define __KLEOPATRA_COMMANDS_COMMAND_H__

#include <QObject>

#include <qwindowdefs.h> // for WId

#include <utils/pimpl_ptr.h>
#include <utils/types.h> // for ExecutionContext

#include <vector>

class QModelIndex;
template <typename T> class QList;
class QAbstractItemView;

namespace GpgME
{
class Key;
}

namespace Kleo
{

class KeyListController;
class AbstractKeyListSortFilterProxyModel;

class Command : public QObject, public ExecutionContext
{
    Q_OBJECT
public:
    explicit Command(KeyListController *parent);
    explicit Command(QAbstractItemView *view, KeyListController *parent);
    explicit Command(const GpgME::Key &key);
    explicit Command(const std::vector<GpgME::Key> &keys);
    ~Command();

    enum Restriction {
        NoRestriction      = 0,
        NeedSelection      = 1,
        OnlyOneKey         = 2,
        NeedSecretKey      = 4,
        MustNotBeSecretKey = 8,
        MustBeOpenPGP      = 16,
        MustBeCMS          = 32,

        // esoteric:
        MayOnlyBeSecretKeyIfOwnerTrustIsNotYetUltimate = 64, // for set-owner-trust

        AnyCardHasNullPin   = 128,
        AnyCardCanLearnKeys = 256,

        MustBeRoot          = 512,
        MustBeTrustedRoot   = 1024 | MustBeRoot,
        MustBeUntrustedRoot = 2048 | MustBeRoot,

        _AllRestrictions_Helper,
        AllRestrictions = 2 * (_AllRestrictions_Helper - 1) - 1
    };

    Q_DECLARE_FLAGS(Restrictions, Restriction)

    static Restrictions restrictions()
    {
        return NoRestriction;
    }

    void setParentWidget(QWidget *widget);
    void setParentWId(WId wid);
    void setView(QAbstractItemView *view);
    void setIndex(const QModelIndex &idx);
    void setIndexes(const QList<QModelIndex> &idx);
    void setKey(const GpgME::Key &key);
    void setKeys(const std::vector<GpgME::Key> &keys);

    void setAutoDelete(bool on);
    bool autoDelete() const;

    void setWarnWhenRunningAtShutdown(bool warn);
    bool warnWhenRunningAtShutdown() const;

public Q_SLOTS:
    void start();
    void cancel();

Q_SIGNALS:
    void info(const QString &message, int timeout = 0);
    void progress(const QString &message, int current, int total);
    void finished();
    void canceled();

private:
    virtual void doStart() = 0;
    virtual void doCancel() = 0;

private:
    /* reimp */ void applyWindowID(QWidget *wid) const;

protected:
    void addTemporaryView(const QString &title, AbstractKeyListSortFilterProxyModel *proxy = 0, const QString &tabToolTip = QString());

protected:
    class Private;
    kdtools::pimpl_ptr<Private> d;
protected:
    explicit Command(Private *pp);
    explicit Command(QAbstractItemView *view, Private *pp);
    explicit Command(const std::vector<GpgME::Key> &keys, Private *pp);
    explicit Command(const GpgME::Key &key, Private *pp);
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Kleo::Command::Restrictions)

#endif /* __KLEOPATRA_COMMANDS_COMMAND_H__ */
