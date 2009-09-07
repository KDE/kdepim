/*
    This file is part of KAddressBook.
    Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>

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

#ifndef DETAILLEDSTYLE_H
#define DETAILLEDSTYLE_H

#include <kabc/addressee.h>

#include "printstyle.h"
#include "kabentrypainter.h"

class QPrinter;

namespace KABPrinting {

class AppearancePage;

class DetailledPrintStyle : public PrintStyle
{
  Q_OBJECT

  public:
    explicit DetailledPrintStyle( PrintingWizard *parent );
    ~DetailledPrintStyle();

    void print( const KABC::Addressee::List &contacts, PrintProgress* );

  protected:
    bool printEntries( const KABC::Addressee::List &contacts, QPrinter *printer,
                       QPainter *painter, const QRect &window );
    bool printEntry( const KABC::Addressee &contact, const QRect &window,
                     QPainter *painter, int top, bool fake, QRect *brect );
  private:
    AppearancePage *mPageAppearance;
    KABEntryPainter *mPainter;
    PrintProgress *mPrintProgress;
};

class DetailledPrintStyleFactory : public PrintStyleFactory
{
  public:
    explicit DetailledPrintStyleFactory( PrintingWizard *parent );

    PrintStyle *create() const;
    QString description() const;
};

}

#endif
