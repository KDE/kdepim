#ifndef KADDRESSBOOKPRINTER_H
#define KADDRESSBOOKPRINTER_H

/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>
              (c) 2002 Mirko Boehm <mirko@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kwizard.h>
#include <kabc/addressbook.h>

class KPrinter;

namespace KABPrinting {

    /** This class defines the abstract interface to the KDE
        Addressbook Printing Wizard. It's main intend is to provide a
        means of using the printing wizard as a product of a (shared
        library) plugin factory.
    */

    class PrintingWizard : public KWizard
    {
    public:
        PrintingWizard(KPrinter *printer,
                       KABC::AddressBook* doc,
                       const QStringList& selection,
                       QWidget *parent=0, const char *name=0);
        ~PrintingWizard();
        virtual void registerStyles()=0;
        virtual void print()=0;
        virtual KPrinter *printer()=0;
        virtual KABC::AddressBook *document()=0;
    protected:
        /** The printer is handed over at construction time and needs
            to be already set up.
            The printer is handed down to the PrintStyle object that
            actually prints.
        */
        KPrinter *mPrinter;
        KABC::AddressBook *mDocument;
        QStringList mSelection;
    };

    PrintingWizard *producePrintingWizard(
        KPrinter *printer,
        KABC::AddressBook* doc,
        const QStringList& selection,
        QWidget *parent, const char *name=0);

}

#endif
