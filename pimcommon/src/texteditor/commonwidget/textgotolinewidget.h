/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#ifndef TEXTGOTOLINEWIDGET_H
#define TEXTGOTOLINEWIDGET_H

#include "pimcommon_export.h"
#include <QWidget>

namespace PimCommon
{
class TextGoToLineWidgetPrivate;
class PIMCOMMON_EXPORT TextGoToLineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TextGoToLineWidget(QWidget *parent = Q_NULLPTR);
    ~TextGoToLineWidget();

    void goToLine();

    void setMaximumLineCount(int max);
Q_SIGNALS:
    void moveToLine(int);

protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotCloseBar();
    void slotGoToLine();

private:
    TextGoToLineWidgetPrivate *const d;
};
}
#endif // TEXTGOTOLINEWIDGET_H
