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


#include "sieveeditortabwidget.h"
#include "ksieveui/editor/sieveeditorhelphtmlwidget.h"

#include <KLocalizedString>

#include <QDebug>

using namespace KSieveUi;
SieveEditorTabWidget::SieveEditorTabWidget(QWidget *parent)
    : KTabWidget(parent)
{
    setTabsClosable(true);
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(slotTabCloseRequested(int)));
}

SieveEditorTabWidget::~SieveEditorTabWidget()
{

}

void SieveEditorTabWidget::slotTabCloseRequested(int index)
{
    //Don't remove first tab.
    if (index > 0) {
        removeTab(index);
    }
}

void SieveEditorTabWidget::slotAddHelpPage(const QString &url)
{
    SieveEditorHelpHtmlWidget *htmlPage = new SieveEditorHelpHtmlWidget;
    connect(htmlPage, SIGNAL(titleChanged(KSieveUi::SieveEditorHelpHtmlWidget*,QString)), this, SLOT(slotTitleChanged(KSieveUi::SieveEditorHelpHtmlWidget*,QString)));
    htmlPage->setHelp(url);
    //TODO customize name
    addTab(htmlPage, i18n("Help"));
}

void SieveEditorTabWidget::slotTitleChanged(KSieveUi::SieveEditorHelpHtmlWidget *widget, const QString &title)
{
    const int index = indexOf(widget);
    if (index != -1) {
        setTabText(index, title);
    }
}

void SieveEditorTabWidget::tabRemoved(int index)
{
    if (count() <= 1) {
        setTabBarHidden(true);
    }
    KTabWidget::tabRemoved(index);
}

void SieveEditorTabWidget::tabInserted(int index)
{
    if (count()>1) {
        setTabBarHidden(false);
    }
    KTabWidget::tabInserted(index);
}

