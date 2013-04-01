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

namespace KSieveUi {
SieveScriptPage::SieveScriptPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout;

    QGroupBox *conditions = new QGroupBox(i18n("Condition"));
    QVBoxLayout *hbox = new QVBoxLayout;
    conditions->setLayout(hbox);
    mScriptConditionLister = new SieveConditionWidgetLister;
    hbox->addWidget(mScriptConditionLister);

    vbox->addWidget(conditions);

    QGroupBox *actions = new QGroupBox(i18n("Condition"));
    hbox = new QVBoxLayout;
    actions->setLayout(hbox);
    mScriptActionLister = new SieveActionWidgetLister;
    hbox->addWidget(mScriptActionLister);
    vbox->addWidget(actions);
    setLayout(vbox);
}

SieveScriptPage::~SieveScriptPage()
{

}

}

#include "sievescriptpage.moc"
