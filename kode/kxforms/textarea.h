/*
    This file is part of KDE.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/
#ifndef KXFORMS_TEXTAREA_H
#define KXFORMS_TEXTAREA_H

#include "guielement.h"

class Q3TextEdit;

namespace KXForms {

class Manager;

class TextArea : public GuiElement
{
  public:
    TextArea( Manager *, const QString &label, QWidget *parent );

    void loadData();
    void saveData();

  private:
    Manager *mManager;

    Q3TextEdit *mEdit;
};

}

#endif
