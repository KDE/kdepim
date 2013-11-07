/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include <KLocale>

#include <QVBoxLayout>
#include <QPushButton>
#include <QWebView>
#include <QTreeWidgetItem>
#include <QWebElement>
#include <QWebFrame>
#include <QHeaderView>

namespace ComposerEditorNG
{
class DomTreeWidgetPrivate
{
public:
    DomTreeWidgetPrivate(QWebView *view, DomTreeWidget *qq)
        : mView(view),
          q(qq)
    {
        QVBoxLayout *lay = new QVBoxLayout;
        treeWidget = new QTreeWidget;
        treeWidget->header()->hide();
        lay->addWidget(treeWidget);
        QPushButton *button = new QPushButton( i18n("Update"));
        q->connect(button, SIGNAL(clicked()), q, SLOT(_k_slotUpdate()));
        lay->addWidget(button);
        q->setLayout(lay);
    }
    void _k_slotUpdate();
    void examineChildElements(const QWebElement &parentElement, QTreeWidgetItem *parentItem);

    QTreeWidget *treeWidget;
    QWebView *mView;
    DomTreeWidget *q;
};

void DomTreeWidgetPrivate::_k_slotUpdate()
{
    treeWidget->clear();

    QWebFrame *frame = mView->page()->mainFrame();
    QWebElement document = frame->documentElement();

    examineChildElements(document, treeWidget->invisibleRootItem());
    treeWidget->expandAll();
}

void DomTreeWidgetPrivate::examineChildElements(const QWebElement &parentElement, QTreeWidgetItem *parentItem)
{
    QWebElement element = parentElement.firstChild();
    while (!element.isNull()) {

        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, element.tagName());
        parentItem->addChild(item);

        const QStringList listAttributes = element.attributeNames();
        Q_FOREACH (const QString& str, listAttributes) {
            QTreeWidgetItem *subItem = new QTreeWidgetItem();
            const QString value = element.attribute(str);
            subItem->setText(0, str + QString::fromLatin1(" (%1)").arg(value));
            item->addChild(subItem);
        }

        examineChildElements(element, item);

        element = element.nextSibling();
    }
}

DomTreeWidget::DomTreeWidget(QWebView *view, QWidget *parent)
    : QWidget(parent),d(new DomTreeWidgetPrivate(view,this))
{
}

DomTreeWidget::~DomTreeWidget()
{
    delete d;
}

}

#include "moc_domtreewidget.cpp"
