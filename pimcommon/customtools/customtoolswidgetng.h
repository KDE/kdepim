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

#ifndef CUSTOMTOOLSWIDGETNG_H
#define CUSTOMTOOLSWIDGETNG_H

#include <QWidget>
#include "pimcommon_export.h"
class KToggleAction;
class KActionCollection;
namespace PimCommon
{
class CustomToolsWidgetNgPrivate;
class PIMCOMMON_EXPORT CustomToolsWidgetNg : public QWidget
{
    Q_OBJECT
public:
    explicit CustomToolsWidgetNg(KActionCollection *ac, QWidget *parent = Q_NULLPTR);
    ~CustomToolsWidgetNg();

    QList<KToggleAction *> actionList() const;

    void setText(const QString &text);

public Q_SLOTS:
    void slotToolsWasClosed();
    void slotActivateView(QWidget *w);

Q_SIGNALS:
    void insertText(const QString &url);
    void toolActivated();

private:
    void initializeView(KActionCollection *ac);
    CustomToolsWidgetNgPrivate *const d;
};
}
#endif // CUSTOMTOOLSWIDGETNG_H
