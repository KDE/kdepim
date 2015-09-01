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

#ifndef CUSTOMTOOLSWIDGET_H
#define CUSTOMTOOLSWIDGET_H

#include "pimcommon/pimcommon_export.h"

#include <QWidget>
class KToggleAction;
class QStackedWidget;
class KActionCollection;
namespace PimCommon
{
class ShortUrlWidget;
class TranslatorWidget;
class CustomToolsWidgetPrivate;
class PIMCOMMON_EXPORT CustomToolsWidget : public QWidget
{
    Q_OBJECT
public:
    enum ToolType {
        TranslatorTool = 0,
        ShortUrlTool = 1
    };

    explicit CustomToolsWidget(QWidget *parent = Q_NULLPTR, KActionCollection *ac = Q_NULLPTR);
    ~CustomToolsWidget();

    void switchToTool(CustomToolsWidget::ToolType type);

    CustomToolsWidget::ToolType toolType() const;

    KToggleAction *action(CustomToolsWidget::ToolType type);

    ShortUrlWidget *shortUrlWidget() const;
    TranslatorWidget *translatorWidget() const;

private Q_SLOTS:
    void slotHideTools();
    void slotVisibleShortUrlTools(bool b);
    void slotVisibleTranslatorTools(bool b);

Q_SIGNALS:
    void shortUrlWasClosed();
    void toolSwitched(PimCommon::CustomToolsWidget::ToolType type);
    void translatorWasClosed();
    void insertShortUrl(const QString &url);

private:
    void customToolWasClosed();
    CustomToolsWidgetPrivate *const d;
};
}
#endif // CUSTOMTOOLSWIDGET_H
