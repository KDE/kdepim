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

#include "sievescriptpage.h"
#include "sieveactionwidgetlister.h"
#include "sieveconditionwidgetlister.h"

#include <KLocale>

#include <QVBoxLayout>
#include <QGroupBox>
#include <QButtonGroup>
#include <QRadioButton>

namespace KSieveUi {
SieveScriptPage::SieveScriptPage(QWidget *parent)
    : QWidget(parent),
      mMatchCondition(AndCondition)
{
    QVBoxLayout *topLayout = new QVBoxLayout;

    QGroupBox *conditions = new QGroupBox(i18n("Conditions"));
    QVBoxLayout *vbox = new QVBoxLayout;

    mMatchAll = new QRadioButton( i18n( "Match a&ll of the following" ), this );
    mMatchAny = new QRadioButton( i18n( "Match an&y of the following" ), this );

    vbox->addWidget(mMatchAll);
    vbox->addWidget(mMatchAny);
    mMatchAll->setChecked( true );
    mMatchAny->setChecked( false );

    QButtonGroup *bg = new QButtonGroup( this );
    bg->addButton( mMatchAll );
    bg->addButton( mMatchAny );

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

SieveScriptPage::~SieveScriptPage()
{
}

SieveScriptPage::MatchCondition SieveScriptPage::matchCondition() const
{
    return mMatchCondition;
}

void SieveScriptPage::slotRadioClicked(QAbstractButton* button)
{
    if (button == mMatchAll) {
        mMatchCondition = AndCondition;
    } else {
        mMatchCondition = OrCondition;
    }
}

void SieveScriptPage::generatedScript(QString &script)
{
    if (mScriptConditionLister->conditionNumber() == 1) {
        script += QLatin1String("if (");
    } else if (mMatchCondition == AndCondition) {
        script += QLatin1String("if allof (");
    } else if (mMatchCondition == OrCondition) {
        script += QLatin1String("if anyof (");
    }
    QStringList lstRequires;
    mScriptConditionLister->generatedScript(script);
    script += QLatin1String(") {\n");
    mScriptActionLister->generatedScript(script, lstRequires);
    script += QLatin1String("}\n");

    QString requires;
    Q_FOREACH (const QString &r, lstRequires) {
        requires += QString::fromLatin1("require \"%1\";\n").arg(r);
    }
    if (!requires.isEmpty()) {
        script.prepend(requires);
    }
}

}

#include "sievescriptpage.moc"
