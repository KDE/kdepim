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

#include <KLocale>
#include <KTreeWidgetSearchLine>
#include <KMenu>

#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWebFrame>

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
    lst << i18n("Address") << i18n("Type");
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
    //TODO
}


void AdBlockBlockableItemsWidget::customContextMenuRequested(const QPoint &)
{
#if 0
    KMenu menu;
    menu.addAction(i18n("copy"),this,SLOT(slotCopyItem()));
    menu.exec(QCursor::pos());
#endif
    //TODO
}

void AdBlockBlockableItemsWidget::slotCopyItem()
{

}


#include "adblockblockableitemswidget.moc"
