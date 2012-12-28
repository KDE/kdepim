/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "domtreewidget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QWebView>
#include <QTreeWidgetItem>
#include <QWebElement>
#include <QWebFrame>

DomTreeWidget::DomTreeWidget(QWebView *view, QWidget *parent)
    : QWidget(parent), mView(view)
{
    QVBoxLayout *lay = new QVBoxLayout;
    mTreeWidget = new QTreeWidget;
    lay->addWidget(mTreeWidget);
    QPushButton *button = new QPushButton(QLatin1String("Update"));
    connect(button, SIGNAL(clicked()), this, SLOT(slotUpdate()));
    lay->addWidget(button);
    setLayout(lay);
}

DomTreeWidget::~DomTreeWidget()
{

}

void DomTreeWidget::slotUpdate()
{
    mTreeWidget->clear();

    QWebFrame *frame = mView->page()->mainFrame();
    QWebElement document = frame->documentElement();

    examineChildElements(document, mTreeWidget->invisibleRootItem());
}

void DomTreeWidget::examineChildElements(const QWebElement &parentElement, QTreeWidgetItem *parentItem)
{
    QWebElement element = parentElement.firstChild();
    while (!element.isNull()) {

        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, element.tagName());
        parentItem->addChild(item);

        examineChildElements(element, item);

        element = element.nextSibling();
    }
}
