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

#ifndef DOMTREEWIDGET_H
#define DOMTREEWIDGET_H

#include "composereditor_export.h"

#include <QWidget>
class QTreeWidget;
class QWebView;

namespace ComposerEditorNG
{
class DomTreeWidgetPrivate;
class COMPOSEREDITORNG_EXPORT DomTreeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DomTreeWidget(QWebView *view, QWidget *parent = 0);
    ~DomTreeWidget();

private:
    friend class DomTreeWidgetPrivate;
    DomTreeWidgetPrivate * const d;
    Q_PRIVATE_SLOT(d, void _k_slotUpdate() )
};
}

#endif // DOMTREEWIDGET_H
