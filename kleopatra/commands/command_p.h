/* -*- mode: c++; c-basic-offset:4 -*-
    commands/command_p.h

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

#ifndef __KLEOPATRA_COMMANDS_COMMAND_P_H__
#define __KLEOPATRA_COMMANDS_COMMAND_P_H__

#include "command.h"
#include "controllers/keylistcontroller.h"
#include "models/keylistmodel.h"

#include <QPointer>
#include <QList>
#include <QModelIndex>

#include <gpgme++/key.h>

class Kleo::Command::Private {
    friend class ::Kleo::Command;
    Command * const q;
public:
    explicit Private( Command * qq );
    ~Private();

    QAbstractItemView * view() const { return view_; }
    AbstractKeyListModel * model() const { return controller_ ? controller_->model() : 0 ; }
    const QList<QModelIndex> & indexes() const { return indexes_; }
    GpgME::Key key() const { return model() && !indexes_.empty() ? model()->key( indexes_.front() ) : GpgME::Key::null ; }
    std::vector<GpgME::Key> keys() const { return model() ? model()->keys( indexes_ ) : std::vector<GpgME::Key>() ; }

private:
    QList<QModelIndex> indexes_;
    QPointer<QAbstractItemView> view_;
    QPointer<KeyListController> controller_;
};

#endif /* __KLEOPATRA_COMMANDS_COMMAND_P_H__ */
