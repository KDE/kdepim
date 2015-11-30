/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef HEADERSTYLEINTERFACE_H
#define HEADERSTYLEINTERFACE_H

#include <QObject>
#include "messageviewer_export.h"
class KToggleAction;
class KActionCollection;
class QAction;
class QActionGroup;
class KActionMenu;
namespace MessageViewer
{
class HeaderStyle;
class HeaderStylePlugin;
class MESSAGEVIEWER_EXPORT HeaderStyleInterface : public QObject
{
    Q_OBJECT
public:
    explicit HeaderStyleInterface(MessageViewer::HeaderStylePlugin *, QObject *parent = Q_NULLPTR);
    virtual ~HeaderStyleInterface();
    QList<KToggleAction * > action() const;
    virtual void createAction(KActionMenu *menu, QActionGroup *actionGroup, KActionCollection *ac) = 0;
    virtual void activateAction() = 0;
    HeaderStylePlugin *headerStylePlugin() const;

Q_SIGNALS:
    void styleChanged(MessageViewer::HeaderStylePlugin *plugin);
    void styleUpdated();

protected Q_SLOTS:
    void slotStyleChanged();
protected:
    void addHelpTextAction(QAction *act, const QString &text);
    void addActionToMenu(KActionMenu *menu, QActionGroup *actionGroup);
    QList<KToggleAction *> mAction;
    HeaderStylePlugin *mHeaderStylePlugin;
};
}
#endif // HEADERSTYLEINTERFACE_H
