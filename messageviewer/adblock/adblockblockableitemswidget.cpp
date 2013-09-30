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

#include "adblockblockableitemswidget.h"
#include "adblockcreatefilterdialog.h"

#include <KLocale>
#include <KTreeWidgetSearchLine>
#include <KMenu>

#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWebFrame>
#include <QPointer>

using namespace MessageViewer;

AdBlockBlockableItemsWidget::AdBlockBlockableItemsWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    setLayout(lay);
    mListItems = new QTreeWidget;

    mListItems->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mListItems, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(customContextMenuRequested(QPoint)));

    QStringList lst;
    lst << i18n("Filter") << i18n("Address") << i18n("Type");
    mListItems->setHeaderLabels(lst);

    KTreeWidgetSearchLine *searchLine = new KTreeWidgetSearchLine(this, mListItems);

    lay->addWidget(searchLine);
    lay->addWidget(mListItems);
}

AdBlockBlockableItemsWidget::~AdBlockBlockableItemsWidget()
{
}

void AdBlockBlockableItemsWidget::setWebFrame(QWebFrame *frame)
{
    mListItems->clear();
    searchBlockableElement(frame);
}

void AdBlockBlockableItemsWidget::searchBlockableElement(QWebFrame *frame)
{
    //TODO
    foreach(QWebFrame *childFrame, frame->childFrames()) {
        searchBlockableElement(childFrame);
    }
}

void AdBlockBlockableItemsWidget::customContextMenuRequested(const QPoint &)
{
    if (!mListItems->currentItem())
        return;

    KMenu menu;
    menu.addAction(i18n("Copy"),this,SLOT(slotCopyItem()));
    menu.addAction(i18n("Block item"),this,SLOT(slotBlockItem()));
    menu.exec(QCursor::pos());
}

void AdBlockBlockableItemsWidget::slotBlockItem()
{
    QTreeWidgetItem *item = mListItems->currentItem();
    if (!item)
        return;

    QPointer<AdBlockCreateFilterDialog> dlg = new AdBlockCreateFilterDialog;
    dlg->setPattern(item->text(Url));
    if (dlg->exec()) {
        //TODO
    }
    //TODO
}

void AdBlockBlockableItemsWidget::slotCopyItem()
{
    QTreeWidgetItem *item = mListItems->currentItem();
    if (!item)
        return;

    //TODO
}


#include "adblockblockableitemswidget.moc"
