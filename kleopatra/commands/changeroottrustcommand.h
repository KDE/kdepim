/* -*- mode: c++; c-basic-offset:4 -*-
    commands/changeroottrustcommand.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2010 Klarälvdalens Datakonsult AB

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

#ifndef __KLEOPATRA_COMMANDS_CHANGEROOTTRUSTCOMMAND_H__
#define __KLEOPATRA_COMMANDS_CHANGEROOTTRUSTCOMMAND_H__

#include <commands/command.h>

#include <gpgme++/key.h>

namespace Kleo
{
namespace Commands
{

class ChangeRootTrustCommand : public Command
{
    Q_OBJECT
public:
    explicit ChangeRootTrustCommand(KeyListController *parent);
    explicit ChangeRootTrustCommand(QAbstractItemView *view, KeyListController *parent);
    explicit ChangeRootTrustCommand(const GpgME::Key &key, KeyListController *parent);
    explicit ChangeRootTrustCommand(const GpgME::Key &key, QAbstractItemView *view, KeyListController *parent);
    ~ChangeRootTrustCommand();

    void setTrust(GpgME::Key::OwnerTrust trust);
    GpgME::Key::OwnerTrust trust() const;

    void setTrustListFile(const QString &file);
    QString trustListFile() const;

    /* reimp */ static Restrictions restrictions()
    {
        return OnlyOneKey | MustBeCMS | MustBeRoot;
    }

private:
    /* reimp */ void doStart();
    /* reimp */ void doCancel();

private:
    class Private;
    inline Private *d_func();
    inline const Private *d_func() const;
    Q_PRIVATE_SLOT(d_func(), void slotOperationFinished())
};

class TrustRootCommand : public ChangeRootTrustCommand
{
public:
    explicit TrustRootCommand(KeyListController *parent)
        : ChangeRootTrustCommand(parent)
    {
        setTrust(GpgME::Key::Ultimate);
    }
    explicit TrustRootCommand(QAbstractItemView *view, KeyListController *parent)
        : ChangeRootTrustCommand(view, parent)
    {
        setTrust(GpgME::Key::Ultimate);
    }
    explicit TrustRootCommand(const GpgME::Key &key, KeyListController *parent)
        : ChangeRootTrustCommand(key, parent)
    {
        setTrust(GpgME::Key::Ultimate);
    }
    explicit TrustRootCommand(const GpgME::Key &key, QAbstractItemView *view, KeyListController *parent)
        : ChangeRootTrustCommand(key, view, parent)
    {
        setTrust(GpgME::Key::Ultimate);
    }

    /* reimp */ static Restrictions restrictions()
    {
        return ChangeRootTrustCommand::restrictions() | MustBeUntrustedRoot;
    }

};

class DistrustRootCommand : public ChangeRootTrustCommand
{
public:
    explicit DistrustRootCommand(KeyListController *parent)
        : ChangeRootTrustCommand(parent)
    {
        setTrust(GpgME::Key::Never);
    }
    explicit DistrustRootCommand(QAbstractItemView *view, KeyListController *parent)
        : ChangeRootTrustCommand(view, parent)
    {
        setTrust(GpgME::Key::Never);
    }
    explicit DistrustRootCommand(const GpgME::Key &key, KeyListController *parent)
        : ChangeRootTrustCommand(key, parent)
    {
        setTrust(GpgME::Key::Never);
    }
    explicit DistrustRootCommand(const GpgME::Key &key, QAbstractItemView *view, KeyListController *parent)
        : ChangeRootTrustCommand(key, view, parent)
    {
        setTrust(GpgME::Key::Never);
    }

    /* reimp */ static Restrictions restrictions()
    {
        return ChangeRootTrustCommand::restrictions() | MustBeTrustedRoot;
    }

};

}
}

#endif /* __KLEOPATRA_COMMANDS_CHANGEROOTTRUSTCOMMAND_H__ */
