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

#include "sievescriptpage.h"
#include "sievescripttabwidget.h"
#include "sieveincludewidget.h"
#include "sieveforeverypartwidget.h"
#include "sieveglobalvariablewidget.h"
#include "sieveeditorgraphicalmodewidget.h"

#include "sievewidgetpageabstract.h"
#include "autocreatescripts/autocreatescriptdialog.h"

#include <KLocalizedString>
#include <KMessageBox>

#include <QVBoxLayout>

namespace KSieveUi
{
SieveScriptPage::SieveScriptPage(QWidget *parent)
    : QWidget(parent),
      mIncludeWidget(0),
      mForEveryPartWidget(0),
      mGlobalVariableWidget(0),
      mBlockIfWidget(0)
{
    QVBoxLayout *topLayout = new QVBoxLayout;
    mTabWidget = new SieveScriptTabWidget;
    connect(mTabWidget, &SieveScriptTabWidget::tabCloseRequested, this, &SieveScriptPage::slotCloseTab);

    if (SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QLatin1String("include"))) {
        mIncludeWidget = new SieveIncludeWidget;
        connect(mIncludeWidget, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
        mTabWidget->addTab(mIncludeWidget, i18n("Includes"));

        mGlobalVariableWidget = new SieveGlobalVariableWidget;
        connect(mGlobalVariableWidget, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
        mTabWidget->addTab(mGlobalVariableWidget, i18n("Global Variable"));
    }

    if (SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QLatin1String("foreverypart"))) {
        mForEveryPartWidget = new SieveForEveryPartWidget;
        connect(mForEveryPartWidget, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
        mTabWidget->addTab(mForEveryPartWidget, i18n("ForEveryPart"));
    }

    mBlockIfWidget = createScriptBlock(SieveScriptBlockWidget::BlockIf);
    mTabWidget->addTab(mBlockIfWidget, blockName(KSieveUi::SieveScriptBlockWidget::BlockIf));
    topLayout->addWidget(mTabWidget);
    mTabWidget->setCurrentWidget(mBlockIfWidget);
    setLayout(topLayout);
}

SieveScriptPage::~SieveScriptPage()
{
}

SieveScriptBlockWidget *SieveScriptPage::addScriptBlock(KSieveUi::SieveWidgetPageAbstract::PageType type)
{
    SieveScriptBlockWidget *blockWidget = createScriptBlock(type);
    mTabWidget->insertTab(mTabWidget->count(), blockWidget, blockName(type));
    mTabWidget->setCurrentWidget(blockWidget);
    return blockWidget;
}

SieveScriptBlockWidget *SieveScriptPage::createScriptBlock(KSieveUi::SieveWidgetPageAbstract::PageType type)
{
    SieveScriptBlockWidget *blockWidget = new SieveScriptBlockWidget;
    connect(blockWidget, SIGNAL(addNewBlock(QWidget*,KSieveUi::SieveWidgetPageAbstract::PageType)), SLOT(slotAddNewBlock(QWidget*,KSieveUi::SieveWidgetPageAbstract::PageType)));
    connect(blockWidget, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
    blockWidget->setPageType(type);
    return blockWidget;
}

bool SieveScriptPage::hasAnElseBlock() const
{
    const int numberOfTab(mTabWidget->count());
    for (int i = 0; i < numberOfTab; ++i) {
        if (static_cast<SieveWidgetPageAbstract *>(mTabWidget->widget(i))->pageType() == SieveScriptBlockWidget::BlockElse) {
            return true;
        }
    }
    return false;
}

void SieveScriptPage::slotAddNewBlock(QWidget *widget, KSieveUi::SieveWidgetPageAbstract::PageType type)
{
    if ((type == KSieveUi::SieveScriptBlockWidget::BlockElse) && hasAnElseBlock()) {
        KMessageBox::error(this, i18n("Script should always have just one \"Else\" block. We cannot add another one."));
        return;
    }
    SieveScriptBlockWidget *blockWidget = createScriptBlock(type);
    if (type == KSieveUi::SieveScriptBlockWidget::BlockElse) { //Insert at the end of tabwidget
        mTabWidget->insertTab(mTabWidget->count(), blockWidget, blockName(type));
    } else {
        mTabWidget->insertTab(mTabWidget->indexOf(widget) + 1, blockWidget, blockName(type));
    }
    mTabWidget->setCurrentWidget(blockWidget);
}

QString SieveScriptPage::blockName(KSieveUi::SieveWidgetPageAbstract::PageType type) const
{
    switch (type) {
    case KSieveUi::SieveScriptBlockWidget::BlockIf:
        return i18n("Main block");
    case KSieveUi::SieveScriptBlockWidget::BlockElsIf:
        return i18n("Block \"Elsif\"");
    case KSieveUi::SieveScriptBlockWidget::BlockElse:
        return i18n("Block \"Else\"");
    default:
        break;
    }
    return QString();
}

void SieveScriptPage::generatedScript(QString &script, QStringList &requires)
{
    QString foreverypartStr;
    QStringList foreverypartRequires;
    if (mForEveryPartWidget) {
        mForEveryPartWidget->generatedScript(foreverypartStr, foreverypartRequires);
        if (!foreverypartStr.isEmpty()) {
            requires << foreverypartRequires;
            script += foreverypartStr + QLatin1Char('\n');
        }
    }
    const int numberOfTab(mTabWidget->count());
    for (int i = 0; i < numberOfTab; ++i) {
        SieveWidgetPageAbstract *page = static_cast<SieveWidgetPageAbstract *>(mTabWidget->widget(i));
        if (page->pageType() != KSieveUi::SieveScriptBlockWidget::ForEveryPart) {
            page->generatedScript(script, requires);
        }
    }
    if (!foreverypartStr.isEmpty()) {
        script += QLatin1String("\n}\n");
    }
}

void SieveScriptPage::slotCloseTab(int index)
{
    mTabWidget->removeTab(index);
    Q_EMIT valueChanged();
}

SieveIncludeWidget *SieveScriptPage::includeWidget() const
{
    return mIncludeWidget;
}

SieveForEveryPartWidget *SieveScriptPage::forEveryPartWidget() const
{
    return mForEveryPartWidget;
}

SieveGlobalVariableWidget *SieveScriptPage::globalVariableWidget() const
{
    return mGlobalVariableWidget;
}

SieveScriptBlockWidget *SieveScriptPage::blockIfWidget() const
{
    return mBlockIfWidget;
}

}

