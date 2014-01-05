/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#ifndef CUSTOMHEADERSETTINGWIDGET_H
#define CUSTOMHEADERSETTINGWIDGET_H

#include "messageviewer_export.h"

#include <QWidget>

namespace PimCommon {
class SimpleStringListEditor;
}

class QRadioButton;
class QButtonGroup;
namespace MessageViewer {

class MESSAGEVIEWER_EXPORT CustomHeaderSettingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CustomHeaderSettingWidget(QWidget *parent = 0);
    ~CustomHeaderSettingWidget();

    void readConfig();
    void writeConfig();

public Q_SLOTS:
    void resetToDefault();

Q_SIGNALS:
    void changed();

private Q_SLOTS:
    void slotHeaderClicked(int);

private:
    QStringList mHeadersToDisplay;
    QStringList mHeadersToHide;
    QRadioButton *mCbHeaderToHide;
    QRadioButton *mCbHeaderToShow;
    PimCommon::SimpleStringListEditor *mHeaders;
    QButtonGroup *mHeaderGroup;
};

}

#endif // CUSTOMHEADERSETTINGWIDGET_H
