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

#ifndef EXTENDATTRIBUTES_H
#define EXTENDATTRIBUTES_H

#include <KDialog>
class QWebElement;

namespace ComposerEditorNG
{
class ExtendAttributesPrivate;
class ExtendAttributes : public KDialog
{
    Q_OBJECT
public:
    enum ExtendType {
        Image,
        Table,
        Cell,
        Link
    };

    explicit ExtendAttributes(const QWebElement& element, ExtendType type,QWidget *parent);
    ~ExtendAttributes();
private:
    friend class ExtendAttributesPrivate;
    ExtendAttributesPrivate * const d;
    Q_PRIVATE_SLOT( d, void _k_slotOkClicked() )
    Q_PRIVATE_SLOT( d, void _k_slotRemoveAttribute() )
};
}

#endif // EXTENDATTRIBUTES_H
