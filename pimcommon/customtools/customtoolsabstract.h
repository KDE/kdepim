/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef CUSTOMTOOLSABSTRACT_H
#define CUSTOMTOOLSABSTRACT_H

#include "pimcommon/pimcommon_export.h"
#include <QObject>
class KToggleAction;
class KActionCollection;
namespace PimCommon
{
class CustomToolsAbstractPrivate;
class PIMCOMMON_EXPORT CustomToolsAbstract : public QObject
{
    Q_OBJECT
public:
    explicit CustomToolsAbstract(QObject *parent = Q_NULLPTR);
    ~CustomToolsAbstract();

    virtual void createAction() = 0;
    virtual KToggleAction *action() const = 0;
    virtual QWidget *createView() = 0;
    virtual QString customToolName() const = 0;

    virtual void setShortcut(KActionCollection *ac);

Q_SIGNALS:
    void toolsWasClosed();
    void activateTool(QWidget *);
    void insertText(const QString &);

private:
    CustomToolsAbstractPrivate *const d;
};
}
#endif // CUSTOMTOOLSABSTRACT_H
