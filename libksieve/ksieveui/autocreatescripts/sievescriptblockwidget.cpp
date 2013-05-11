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

#include "sievescriptblockwidget.h"
#include "sieveactionwidgetlister.h"
#include "sieveconditionwidgetlister.h"

#include <KLocale>

#include <QVBoxLayout>
#include <QGroupBox>
#include <QButtonGroup>
#include <QRadioButton>

namespace KSieveUi {
SieveScriptBlockWidget::SieveScriptBlockWidget(QWidget *parent)
    : QWidget(parent),
      mType(BlockIf),
      mMatchCondition(AndCondition)
{
    QVBoxLayout *topLayout = new QVBoxLayout;

    QGroupBox *conditions = new QGroupBox(i18n("Conditions"));
    QVBoxLayout *vbox = new QVBoxLayout;

    mAllMessageRBtn = new QRadioButton( i18n( "Match all messages" ), this );
    mMatchAll = new QRadioButton( i18n( "Match a&ll of the following" ), this );
    mMatchAny = new QRadioButton( i18n( "Match an&y of the following" ), this );

    vbox->addWidget(mMatchAll);
    vbox->addWidget(mMatchAny);
    vbox->addWidget(mAllMessageRBtn);
    mMatchAll->setChecked( true );
    mMatchAny->setChecked( false );
    mAllMessageRBtn->setChecked( false );

    QButtonGroup *bg = new QButtonGroup( this );
    bg->addButton( mMatchAll );
    bg->addButton( mMatchAny );
    bg->addButton(mAllMessageRBtn);

    connect( bg, SIGNAL(buttonClicked(QAbstractButton*)),
             this, SLOT(slotRadioClicked(QAbstractButton*)) );
    conditions->setLayout(vbox);

    mScriptConditionLister = new SieveConditionWidgetLister;
    vbox->addWidget(mScriptConditionLister);

    topLayout->addWidget(conditions);

    QGroupBox *actions = new QGroupBox(i18n("Actions"));
    vbox = new QVBoxLayout;
    actions->setLayout(vbox);
    mScriptActionLister = new SieveActionWidgetLister;
    vbox->addWidget(mScriptActionLister);
    topLayout->addWidget(actions);
    setLayout(topLayout);
}

SieveScriptBlockWidget::~SieveScriptBlockWidget()
{

}

void SieveScriptBlockWidget::setBlockType(BlockType type)
{
    if (mType != type) {
        mType = type;
        //TODO update GUI
    }
}

SieveScriptBlockWidget::BlockType SieveScriptBlockWidget::blockType() const
{
    return mType;
}

SieveScriptBlockWidget::MatchCondition SieveScriptBlockWidget::matchCondition() const
{
    return mMatchCondition;
}

void SieveScriptBlockWidget::slotRadioClicked(QAbstractButton* button)
{
    if (button == mMatchAll) {
        mMatchCondition = AndCondition;
    } else if (button == mMatchAny) {
        mMatchCondition = OrCondition;
    } else {
        mMatchCondition = AllCondition;
    }
    mScriptConditionLister->setEnabled(mMatchCondition != AllCondition);
}

void SieveScriptBlockWidget::generatedScript(QString &script, QStringList &requires)
{
    if (mMatchCondition == AllCondition) {
        script += QLatin1String("if true {\n");
    } else {
        QString conditionStr;
        int numberOfCondition = 0;
        mScriptConditionLister->generatedScript(conditionStr, numberOfCondition, requires);
        const bool hasUniqCondition = (numberOfCondition == 1);
        QString filterStr;
        QString blockStr;
        switch (mType) {
        case BlockIf:
            blockStr = QLatin1String("if ");
            break;
        case BlockElsIf:
            blockStr = QLatin1String("elsif ");
            break;
        case BlockElse:
            blockStr = QLatin1String("else");
            break;
        }

        if (mType == BlockElse) {
            filterStr += blockStr;
        } else if (hasUniqCondition == 1) {
            filterStr += blockStr;
        }

        if (mMatchCondition == AndCondition) {
            filterStr += QLatin1String("allof (");
        } else if (mMatchCondition == OrCondition) {
            filterStr += QLatin1String("anyof (");
        }

        if (conditionStr.isEmpty()) {
            return;
        } else {
            script += filterStr + conditionStr;
        }
        if (hasUniqCondition)
            script += QLatin1String("{\n");
        else
            script += QLatin1String(")\n{\n");
    }
    mScriptActionLister->generatedScript(script, requires);
    script += QLatin1String("}\n");
}



}

#include "sievescriptblockwidget.moc"
