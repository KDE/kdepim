/*
  This file is part of KAddressBook.
  Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>
                     2002 Mike Pilone <mpilone@slac.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef MIKESSTYLE_H
#define MIKESSTYLE_H

#include "printstyle.h"

namespace KABPrinting
{

class PrintProgress;

class MikesStyle : public PrintStyle
{
    Q_OBJECT

public:
    explicit MikesStyle(PrintingWizard *parent);
    ~MikesStyle();

    void print(const KContacts::Addressee::List &, PrintProgress *) Q_DECL_OVERRIDE;
};

class MikesStyleFactory : public PrintStyleFactory
{
public:
    explicit MikesStyleFactory(PrintingWizard *parent);

    PrintStyle *create() const Q_DECL_OVERRIDE;
    QString description() const Q_DECL_OVERRIDE;
};

}

#endif
