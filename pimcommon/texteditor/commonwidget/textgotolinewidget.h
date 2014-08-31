/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

class QSpinBox;
class QPushButton;
namespace PimCommon {
class PIMCOMMON_EXPORT TextGoToLineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TextGoToLineWidget(QWidget *parent=0);
    ~TextGoToLineWidget();

    void goToLine();

Q_SIGNALS:
    void moveToLine(int);


protected:
    bool event(QEvent* e);
    void showEvent(QShowEvent *e);

private slots:
    void slotCloseBar();
    void slotGoToLine();

private:
    QSpinBox *mSpinbox;
    QPushButton *mGoToLine;
};
}
#endif // TEXTGOTOLINEWIDGET_H
