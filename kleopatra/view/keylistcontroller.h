/* -*- mode: c++; c-basic-offset:4 -*-
    controllers/keylistcontroller.h

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
#ifndef __KLEOPATRA_CONTROLLERS_KEYLISTCONTROLLER_H__
#define __KLEOPATRA_CONTROLLERS_KEYLISTCONTROLLER_H__

#include <QObject>

#include <commands/command.h>

#include <utils/pimpl_ptr.h>

#include <vector>

class QAbstractItemView;
class QAction;
class QPoint;
class QItemSelectionModel;
class KActionCollection;

namespace Kleo
{

class AbstractKeyListModel;
class Command;
class TabWidget;

class KeyListController : public QObject
{
    Q_OBJECT
public:
    explicit KeyListController(QObject *parent = 0);
    ~KeyListController();

    std::vector<QAbstractItemView *> views() const;

    void setFlatModel(AbstractKeyListModel *model);
    AbstractKeyListModel *flatModel() const;

    void setHierarchicalModel(AbstractKeyListModel *model);
    AbstractKeyListModel *hierarchicalModel() const;

    void setParentWidget(QWidget *parent);
    QWidget *parentWidget() const;

    QAbstractItemView *currentView() const;

    void setTabWidget(TabWidget *tabs);
    TabWidget *tabWidget() const;

    void registerCommand(Command *cmd);

    void createActions(KActionCollection *collection);

    template <typename T_Command>
    void registerActionForCommand(QAction *action)
    {
        this->registerAction(action, T_Command::restrictions(), &KeyListController::template create<T_Command>);
    }

    void enableDisableActions(const QItemSelectionModel *sm) const;

    bool hasRunningCommands() const;
    bool shutdownWarningRequired() const;

private:
    void registerAction(QAction *action, Command::Restrictions restrictions , Command * (*create)(QAbstractItemView *, KeyListController *));

    template <typename T_Command>
    static Command *create(QAbstractItemView *v, KeyListController *c)
    {
        return new T_Command(v, c);
    }

public Q_SLOTS:
    void addView(QAbstractItemView *view);
    void removeView(QAbstractItemView *view);
    void setCurrentView(QAbstractItemView *view);

    void cancelCommands();
    void updateConfig();

Q_SIGNALS:
    void progress(int current, int total);
    void message(const QString &msg, int timeout = 0);

    void commandsExecuting(bool);

    void contextMenuRequested(QAbstractItemView *view, const QPoint &p);

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;

    Q_PRIVATE_SLOT(d, void slotDestroyed(QObject *))
    Q_PRIVATE_SLOT(d, void slotDoubleClicked(QModelIndex))
    Q_PRIVATE_SLOT(d, void slotActivated(QModelIndex))
    Q_PRIVATE_SLOT(d, void slotSelectionChanged(QItemSelection, QItemSelection))
    Q_PRIVATE_SLOT(d, void slotContextMenu(QPoint))
    Q_PRIVATE_SLOT(d, void slotCommandFinished())
    Q_PRIVATE_SLOT(d, void slotAddKey(GpgME::Key))
    Q_PRIVATE_SLOT(d, void slotAboutToRemoveKey(GpgME::Key))
    Q_PRIVATE_SLOT(d, void slotProgress(QString, int, int))
    Q_PRIVATE_SLOT(d, void slotActionTriggered())
    Q_PRIVATE_SLOT(d, void slotCurrentViewChanged(QAbstractItemView *))
};

}

#endif /* __KLEOPATRA_CONTROLLERS_KEYLISTCONTROLLER_H__ */
