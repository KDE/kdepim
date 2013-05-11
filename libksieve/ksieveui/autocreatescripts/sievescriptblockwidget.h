/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include <QWidget>

class QRadioButton;
class QGroupBox;
class QAbstractButton;
class QHBoxLayout;
class KComboBox;
class KPushButton;

namespace KSieveUi {
class SieveConditionWidgetLister;
class SieveActionWidgetLister;

class SieveScriptBlockWidget : public QWidget
{
    Q_OBJECT
public:
    enum BlockType {
        BlockIf = 0,
        BlockElsIf = 1,
        BlockElse = 2
    };

    enum MatchCondition {
        OrCondition,
        AndCondition,
        AllCondition
    };

    explicit SieveScriptBlockWidget(QWidget *parent = 0);
    ~SieveScriptBlockWidget();

    void setBlockType(BlockType type);
    BlockType blockType() const;

    void generatedScript(QString &script, QStringList &requires);

    MatchCondition matchCondition() const;

Q_SIGNALS:
    void addNewBlock(QWidget *widget, KSieveUi::SieveScriptBlockWidget::BlockType type);

private Q_SLOTS:
    void slotRadioClicked(QAbstractButton*);
    void slotAddBlock();

private:
    BlockType mType;
    MatchCondition mMatchCondition;
    QGroupBox *mConditions;
    SieveConditionWidgetLister *mScriptConditionLister;
    SieveActionWidgetLister *mScriptActionLister;
    QRadioButton *mMatchAll;
    QRadioButton *mMatchAny;
    QRadioButton *mAllMessageRBtn;
    KComboBox *mNewBlockType;
    KPushButton *mAddBlockType;
    QHBoxLayout *mNewBlockLayout;
};
}

#endif // SIEVESCRIPTBLOCKWIDGET_H
