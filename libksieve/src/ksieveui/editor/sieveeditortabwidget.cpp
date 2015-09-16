/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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
#include <KStringHandler>

#include <QTabBar>
#include <QUrl>

using namespace KSieveUi;
SieveEditorTabWidget::SieveEditorTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    setTabsClosable(true);
    connect(this, &SieveEditorTabWidget::tabCloseRequested, this, &SieveEditorTabWidget::slotTabCloseRequested);
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

QUrl SieveEditorTabWidget::currentHelpUrl() const
{
    SieveEditorHelpHtmlWidget *page = qobject_cast<SieveEditorHelpHtmlWidget *>(currentWidget());
    if (page) {
        return page->currentUrl();
    }
    return QUrl();
}

QString SieveEditorTabWidget::currentHelpTitle() const
{
    SieveEditorHelpHtmlWidget *page = qobject_cast<SieveEditorHelpHtmlWidget *>(currentWidget());
    if (page) {
        return page->title();
    }
    return QString();
}

void SieveEditorTabWidget::slotAddHelpPage(const QUrl &url)
{
    for (int i = 0; i < count(); ++i) {
        SieveEditorHelpHtmlWidget *page = qobject_cast<SieveEditorHelpHtmlWidget *>(widget(i));
        if (page && page->currentUrl() == url) {
            setCurrentIndex(i);
            return;
        }
    }
    SieveEditorHelpHtmlWidget *htmlPage = new SieveEditorHelpHtmlWidget;
    connect(htmlPage, &SieveEditorHelpHtmlWidget::titleChanged, this, &SieveEditorTabWidget::slotTitleChanged);
    connect(htmlPage, &SieveEditorHelpHtmlWidget::progressIndicatorPixmapChanged, this, &SieveEditorTabWidget::slotProgressIndicatorPixmapChanged);
    connect(htmlPage, &SieveEditorHelpHtmlWidget::loadFinished, this, &SieveEditorTabWidget::slotLoadFinished);
    htmlPage->openUrl(url);
    const int index = addTab(htmlPage, i18n("Help"));
    setCurrentIndex(index);
}

void SieveEditorTabWidget::slotLoadFinished(KSieveUi::SieveEditorHelpHtmlWidget *widget, bool success)
{
    const int index = indexOf(widget);
    if (index != -1) {
        setTabIcon(index, QIcon());
    }
    if (!success) {
        setTabText(index, i18n("Error during load page about %1", widget->title()));
    }
}

void SieveEditorTabWidget::slotProgressIndicatorPixmapChanged(KSieveUi::SieveEditorHelpHtmlWidget *widget, const QPixmap &pixmap)
{
    const int index = indexOf(widget);
    if (index != -1) {
        setTabIcon(index, QIcon(pixmap));
    }
}

void SieveEditorTabWidget::slotTitleChanged(KSieveUi::SieveEditorHelpHtmlWidget *widget, const QString &title)
{
    const int index = indexOf(widget);
    if (index != -1) {
        const QString troncateTitle = KStringHandler::lsqueeze(title, 30);
        setTabText(index, i18n("Help about: %1", troncateTitle));
        setTabToolTip(index, title);
    }
}

void SieveEditorTabWidget::tabRemoved(int index)
{
    if (count() <= 1) {
        tabBar()->hide();
    }
    QTabWidget::tabRemoved(index);
}

void SieveEditorTabWidget::tabInserted(int index)
{
    if (count() > 1) {
        tabBar()->show();
    }
    QTabWidget::tabInserted(index);
    tabBar()->tabButton(0, QTabBar::RightSide)->setEnabled(false);
}

