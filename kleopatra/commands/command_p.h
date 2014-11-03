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
#include "view/keylistcontroller.h"
#include "models/keylistmodel.h"

#include <KMessageBox>

#include <QAbstractItemView>
#include <QPointer>
#include <QList>
#include <QModelIndex>

#include <gpgme++/key.h>

#include <algorithm>
#include <iterator>

class Kleo::Command::Private
{
    friend class ::Kleo::Command;
protected:
    Command *const q;
public:
    explicit Private(Command *qq, KeyListController *controller);
    virtual ~Private();

    QAbstractItemView *view() const
    {
        return view_;
    }
    QWidget *parentWidgetOrView() const
    {
        if (parentWidget_) {
            return parentWidget_;
        } else {
            return view_;
        }
    }
    KeyListModelInterface *model() const
    {
        return view_ ? dynamic_cast<KeyListModelInterface *>(view_->model()) : 0 ;
    }
    KeyListController *controller() const
    {
        return controller_;
    }
    QList<QModelIndex> indexes() const
    {
        QList<QModelIndex> result;
        std::copy(indexes_.begin(), indexes_.end(), std::back_inserter(result));
        return result;
    }
    GpgME::Key key() const
    {
        return keys_.empty() ? model() && !indexes_.empty() ? model()->key(indexes_.front()) : GpgME::Key::null : keys_.front() ;
    }
    std::vector<GpgME::Key> keys() const
    {
        return keys_.empty() ? model() ? model()->keys(indexes()) : std::vector<GpgME::Key>() : keys_ ;
    }

    void finished()
    {
        emit q->finished();
        if (autoDelete) {
            q->deleteLater();
        }
    }

    void canceled()
    {
        emit q->canceled();
        finished();
    }

    void error(const QString &text, const QString &caption = QString(), KMessageBox::Options options = KMessageBox::Notify) const
    {
        if (parentWId) {
            KMessageBox::errorWId(parentWId, text, caption, options);
        } else {
            KMessageBox::error(parentWidgetOrView(), text, caption, options);
        }
    }
    void information(const QString &text, const QString &caption = QString(), const QString &dontShowAgainName = QString(), KMessageBox::Options options = KMessageBox::Notify) const
    {
        if (parentWId) {
            KMessageBox::informationWId(parentWId, text, caption, dontShowAgainName, options);
        } else {
            KMessageBox::information(parentWidgetOrView(), text, caption, dontShowAgainName, options);
        }
    }

    void applyWindowID(QWidget *w) const
    {
        return q->applyWindowID(w);
    }

private:
    bool autoDelete : 1;
    bool warnWhenRunningAtShutdown : 1;
    std::vector<GpgME::Key> keys_;
    QList<QPersistentModelIndex> indexes_;
    QPointer<QAbstractItemView> view_;
    QPointer<QWidget> parentWidget_;
    WId parentWId;
    QPointer<KeyListController> controller_;
};

#endif /* __KLEOPATRA_COMMANDS_COMMAND_P_H__ */
