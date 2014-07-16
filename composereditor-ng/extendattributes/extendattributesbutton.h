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

#ifndef EXTENDATTRIBUTESBUTTON_H
#define EXTENDATTRIBUTESBUTTON_H

#include "extendattributesdialog.h"

#include <QPushButton>

class QWebElement;

namespace ComposerEditorNG
{

class ExtendAttributesButtonPrivate;
class ExtendAttributesButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ExtendAttributesButton(const QWebElement &element, ComposerEditorNG::ExtendAttributesDialog::ExtendType type, QWidget *parent);
    ~ExtendAttributesButton();

Q_SIGNALS:
    void webElementChanged();

private:
    friend class ExtendAttributesButtonPrivate;
    ExtendAttributesButtonPrivate * const d;
    Q_PRIVATE_SLOT(d, void _k_slotClicked() )
};
}

#endif // EXTENDATTRIBUTESBUTTON_H
