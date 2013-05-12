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
#include "sievescripttabwidget.h"

#include <KLocale>
#include <KTabWidget>

#include <QVBoxLayout>

namespace KSieveUi {
SieveScriptPage::SieveScriptPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *topLayout = new QVBoxLayout;
    mTabWidget = new SieveScriptTabWidget;
    connect(mTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(slotCloseTab(int)));
    SieveScriptBlockWidget *blockWidget = createScriptBlock(SieveScriptBlockWidget::BlockIf);
    mTabWidget->addTab(blockWidget, i18n("Main block"));
    topLayout->addWidget(mTabWidget);
    setLayout(topLayout);
}

SieveScriptPage::~SieveScriptPage()
{
}

SieveScriptBlockWidget *SieveScriptPage::createScriptBlock(KSieveUi::SieveScriptBlockWidget::BlockType type)
{
    SieveScriptBlockWidget *blockWidget = new SieveScriptBlockWidget;
    connect(blockWidget, SIGNAL(addNewBlock(QWidget*,KSieveUi::SieveScriptBlockWidget::BlockType)), SLOT(slotAddNewBlock(QWidget*,KSieveUi::SieveScriptBlockWidget::BlockType)));
    blockWidget->setBlockType(type);
    return blockWidget;
}

void SieveScriptPage::slotAddNewBlock(QWidget* widget,KSieveUi::SieveScriptBlockWidget::BlockType type)
{
    SieveScriptBlockWidget *blockWidget = createScriptBlock(type);
    mTabWidget->insertTab(mTabWidget->indexOf(widget)+1, blockWidget, i18n("Block"));
}

void SieveScriptPage::generatedScript(QString &script, QStringList &requires)
{
    const int numberOfTab(mTabWidget->count());
    for (int i = 0; i < numberOfTab; ++i) {
        static_cast<SieveScriptBlockWidget*>(mTabWidget->widget(i))->generatedScript(script, requires);
    }
}

void SieveScriptPage::slotCloseTab(int index)
{
    mTabWidget->removeTab(index);
}

}

#include "sievescriptpage.moc"
