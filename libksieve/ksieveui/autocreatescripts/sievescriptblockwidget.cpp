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
#include <KComboBox>
#include <KPushButton>

#include <QDomElement>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QLabel>
#include <QDebug>


namespace KSieveUi {
SieveScriptBlockWidget::SieveScriptBlockWidget(QWidget *parent)
    : SieveWidgetPageAbstract(parent),
      mMatchCondition(AndCondition)
{
    QVBoxLayout *topLayout = new QVBoxLayout;

    mConditions = new QGroupBox(i18n("Conditions"));
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
    mConditions->setLayout(vbox);

    mScriptConditionLister = new SieveConditionWidgetLister;
    vbox->addWidget(mScriptConditionLister);

    topLayout->addWidget(mConditions,0, Qt::AlignTop);

    QGroupBox *actions = new QGroupBox(i18n("Actions"));
    vbox = new QVBoxLayout;
    actions->setLayout(vbox);
    mScriptActionLister = new SieveActionWidgetLister;
    vbox->addWidget(mScriptActionLister,0, Qt::AlignTop);
    topLayout->addWidget(actions);

    QHBoxLayout *newBlockLayout = new QHBoxLayout;
    QLabel *lab = new QLabel(i18n("Add new block:"));
    newBlockLayout->addWidget(lab);
    mNewBlockType = new KComboBox;
    newBlockLayout->addWidget(mNewBlockType);
    mNewBlockType->addItem(i18n("\"elsif\" block"));
    mNewBlockType->addItem(i18n("\"else\" block"));

    mAddBlockType = new KPushButton;
    mAddBlockType->setIcon( KIcon( QLatin1String("list-add") ) );
    mAddBlockType->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    newBlockLayout->addWidget(mAddBlockType);
    connect(mAddBlockType, SIGNAL(clicked(bool)), SLOT(slotAddBlock()));

    topLayout->addLayout(newBlockLayout);

    setLayout(topLayout);
}

SieveScriptBlockWidget::~SieveScriptBlockWidget()
{
}

void SieveScriptBlockWidget::slotAddBlock()
{
    KSieveUi::SieveWidgetPageAbstract::PageType type;
    switch(mNewBlockType->currentIndex()) {
    case 0:
        type = BlockElsIf;
        break;
    case 1:
        type = BlockElse;
        break;
    }

    Q_EMIT addNewBlock(this, type);
}

void SieveScriptBlockWidget::setPageType(PageType type)
{

    if (pageType() != type) {
        SieveWidgetPageAbstract::setPageType(type);
        switch(type) {
        case BlockIf:
            mAllMessageRBtn->show();
            mConditions->show();
            mAddBlockType->setEnabled(true);
            mNewBlockType->setEnabled(true);
            break;
        case BlockElsIf:
            mAllMessageRBtn->hide();
            mConditions->show();
            mAddBlockType->setEnabled(true);
            mNewBlockType->setEnabled(true);
            break;
        case BlockElse:
            mAllMessageRBtn->hide();
            mConditions->hide();
            mAddBlockType->setEnabled(false);
            mNewBlockType->setEnabled(false);
            break;
        default:
            break;
        }
    }
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
    mNewBlockType->setEnabled(mMatchCondition != AllCondition);
}

void SieveScriptBlockWidget::generatedScript(QString &script, QStringList &requires)
{
    bool onlyActions = false;
    if (mMatchCondition == AllCondition) {
        onlyActions = true;
        //Just actions type
        //script += QLatin1String("if true {\n");
    } else if (pageType() == BlockElse) {
        script += QLatin1String("else {\n");
    } else {
        QString conditionStr;
        int numberOfCondition = 0;
        mScriptConditionLister->generatedScript(conditionStr, numberOfCondition, requires);
        const bool hasUniqCondition = (numberOfCondition == 1);
        QString filterStr;
        QString blockStr;
        switch (pageType()) {
        case BlockIf:
            blockStr = QLatin1String("if ");
            break;
        case BlockElsIf:
            blockStr = QLatin1String("elsif ");
            break;
        case BlockElse:
            break;
        default:
            //We can got here.
            break;
        }

        if (hasUniqCondition == 1) {
            filterStr += blockStr;
        } else if (mMatchCondition == AndCondition) {
            filterStr += blockStr + QLatin1String("allof (");
        } else if (mMatchCondition == OrCondition) {
            filterStr += blockStr + QLatin1String("anyof (");
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
    mScriptActionLister->generatedScript(script, requires, onlyActions);
    if (!onlyActions) {
        script += QLatin1String("} ");
    }
}

void SieveScriptBlockWidget::loadScript(const QDomElement &element, bool onlyActions)
{
    if (onlyActions) {
        mScriptActionLister->loadScript(element, onlyActions);
        mAllMessageRBtn->setChecked(true);
        mMatchCondition = AllCondition;
        mScriptConditionLister->setEnabled(mMatchCondition != AllCondition);
        mNewBlockType->setEnabled(mMatchCondition != AllCondition);
    } else {
        bool uniqueTest = false;
        QDomNode node = element.firstChild();
        while (!node.isNull()) {
            QDomElement e = node.toElement();
            if (!e.isNull()) {
                const QString tagName = e.tagName();
                bool notCondition = false;
                if (tagName == QLatin1String("test")) {
                    if (e.hasAttribute(QLatin1String("name"))) {
                        const QString typeCondition = e.attribute(QLatin1String("name"));
                        if (typeCondition == QLatin1String("anyof")) {
                            qDebug()<<" Anyof";
                            mMatchCondition = OrCondition;
                            mMatchAny->setChecked(true);
                        } else if (typeCondition == QLatin1String("allof")) {
                            qDebug()<<" Allof";
                            mMatchAll->setChecked(true);
                            mMatchCondition = AndCondition;
                        } else { //true;
                            if (typeCondition == QLatin1String("not")) {
                                qDebug()<<" NOT condition";
                                notCondition = true;
                            }
                            uniqueTest = true;
                            mMatchCondition = OrCondition;
                            mMatchAny->setChecked(true);
                            qDebug()<<" TRUE";
                        }
                        mScriptConditionLister->setEnabled(mMatchCondition != AllCondition);
                        mNewBlockType->setEnabled(mMatchCondition != AllCondition);
                    }
                    //Conditions
                    mScriptConditionLister->loadScript(e, uniqueTest, notCondition);
                } else if (tagName == QLatin1String("block")) {
                    //Actions
                    mScriptActionLister->loadScript(e);
                }
                qDebug()<<" e.tag"<<e.tagName();
            }
            node = node.nextSibling();
        }
    }
}


}

#include "sievescriptblockwidget.moc"
