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

#ifndef DOMTREEWIDGET_H
#define DOMTREEWIDGET_H

#include <QWidget>
class QTreeWidget;
class QWebView;
class QTreeWidgetItem;
class QWebElement;

class DomTreeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DomTreeWidget(QWebView *view, QWidget *parent = 0);
    ~DomTreeWidget();
private Q_SLOTS:
    void slotUpdate();
private:
    void examineChildElements(const QWebElement &parentElement, QTreeWidgetItem *parentItem);
    QTreeWidget *mTreeWidget;
    QWebView *mView;
};

#endif // DOMTREEWIDGET_H
