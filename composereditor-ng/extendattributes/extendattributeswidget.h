/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef EXTENDATTRIBUTESWIDGET_H
#define EXTENDATTRIBUTESWIDGET_H

#include <QWidget>

#include "extendattributesdialog.h"

namespace ComposerEditorNG
{
class ExtendAttributesWidgetPrivate;

class ExtendAttributesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ExtendAttributesWidget(const QWebElement &element, ExtendAttributesDialog::SettingsType settings, ExtendAttributesDialog::ExtendType type, QWidget *parent);
    ~ExtendAttributesWidget();

    void changeAttributes();

private:
    friend class ExtendAttributesWidgetPrivate;
    ExtendAttributesWidgetPrivate *const d;
    Q_PRIVATE_SLOT(d, void _k_slotRemoveAttribute())
    Q_PRIVATE_SLOT(d, void _k_attributeChanged(const QString &))
    Q_PRIVATE_SLOT(d, void _k_slotCurrentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *))
    Q_PRIVATE_SLOT(d, void _k_attributeValueChanged(const QString &))
    Q_PRIVATE_SLOT(d, void _k_attributeLineEditChanged(const QString &))
};
}

#endif // EXTENDATTRIBUTESWIDGET_H
