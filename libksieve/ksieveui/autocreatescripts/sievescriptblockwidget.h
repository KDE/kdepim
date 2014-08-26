/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef SIEVESCRIPTBLOCKWIDGET_H
#define SIEVESCRIPTBLOCKWIDGET_H

#include "sievewidgetpageabstract.h"

class QRadioButton;
class QGroupBox;
class QAbstractButton;
class KComboBox;
class QPushButton;
class QDomElement;

namespace KSieveUi
{
class SieveConditionWidgetLister;
class SieveActionWidgetLister;

class SieveScriptBlockWidget : public SieveWidgetPageAbstract
{
    Q_OBJECT
public:
    enum MatchCondition {
        OrCondition,
        AndCondition,
        AllCondition
    };

    explicit SieveScriptBlockWidget(QWidget *parent = 0);
    ~SieveScriptBlockWidget();

    void setPageType(PageType type);

    void generatedScript(QString &script, QStringList &requires);

    MatchCondition matchCondition() const;

    void loadScript(const QDomElement &element, bool onlyActions, QString &error);

Q_SIGNALS:
    void addNewBlock(QWidget *widget, KSieveUi::SieveWidgetPageAbstract::PageType type);

private Q_SLOTS:
    void slotRadioClicked(QAbstractButton *);
    void slotAddBlock();

private:
    void updateWidget();
    void updateCondition();
    MatchCondition mMatchCondition;
    QGroupBox *mConditions;
    SieveConditionWidgetLister *mScriptConditionLister;
    SieveActionWidgetLister *mScriptActionLister;
    QRadioButton *mMatchAll;
    QRadioButton *mMatchAny;
    QRadioButton *mAllMessageRBtn;
    KComboBox *mNewBlockType;
    QPushButton *mAddBlockType;
};
}

#endif // SIEVESCRIPTBLOCKWIDGET_H
