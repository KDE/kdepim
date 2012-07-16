/*
  This file is part of KAddressBook.
  Copyright (c) 2011 Mario Scheel <zweistein12@gmx.de>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef COMPACTSTYLE_H
#define COMPACTSTYLE_H

#include "printstyle.h"

namespace KABPrinting {

class CompactStyleForm;

class CompactStyle : public PrintStyle
{
  Q_OBJECT

  public:
    CompactStyle( PrintingWizard *parent );
    ~CompactStyle();

    /**
     * prints the contacts
     */
    void print( const KABC::Addressee::List &, PrintProgress * );

  private:
    QString contactsToHtml( const KABC::Addressee::List &contacts ) const;

    CompactStyleForm *mPageSettings;

    /**
     * various settings
     */
    bool withAlternating;
    bool withBusinessAddress;
    bool withHomeAddress;
    bool withBirthday;
    bool withEMail;
    QColor firstColor;
    QColor secondColor;

  private Q_SLOTS:
    /**
     * Enable or disable the controls for color selection. The colors
     * are needed for alternating background color of the rows.
     */
    void setAlternatingColors();
};

class CompactStyleFactory : public PrintStyleFactory
{
  public:
    explicit CompactStyleFactory( PrintingWizard *parent );

    PrintStyle *create() const;
    QString description() const;
};

}

#endif
