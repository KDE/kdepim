/* -*- C++ -*-
   This file declares the printing wizard. See
   ../kaddressbookprinting.* for details.

   the KDE addressbook

   $ Author: Mirko Boehm $
   $ Copyright: (C) 1996-2002, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Revision$
*/

#ifndef PRINTINGWIZARD_H
#define PRINTINGWIZARD_H

#include <qlist.h>
#include <qstringlist.h>

#include "../kaddressbookprinter.h"
#include "kabc/addressbook.h"
#include "printstyle.h"

// ----- the general page:
#include "printingwizard_base.h"

class KPrinter;
class QVBoxLayout;

namespace KABPrinting {

    /** The PrintingWizard combines pages common for all print styles
        and those provided by the respective style.
    */
    class PrintingWizardImpl : public PrintingWizard
    {
        Q_OBJECT
    public:
        /** Construct a printing wizard. Give the document
        (addressbook instance) to print.
        */
        PrintingWizardImpl(KPrinter *printer,
                           KABC::AddressBook* doc,
                           const QStringList& selection,
                           QWidget *parent=0, const char *name=0);
        ~PrintingWizardImpl();
        /** Modify this method to add a new PrintStyle.
         */
        void registerStyles();
        /** Perform the actual printing. */
        void print();
        /** Retrieve the document object. */
        KABC::AddressBook *document();
        /** Retrieve the printer to be used. */
        KPrinter* printer();
    protected slots:
        /** A print style has been selected. The argument is the index
            in the cbStyle combo and in styles.
        */
        void slotStyleSelected(int);
    protected:
        QPtrList<PrintStyleFactory> styleFactories;
        QPtrList<PrintStyle> mStyleList;
        PrintStyle *style;
        /** The general page. */
        BasicPage *mBasicPage;
        QVBoxLayout* pageLayout;
        /** Overloaded accept slot. This is used to do the actual
            printing without having the wizard disappearing
            before. What happens is actually up to the print style,
            since it does the printing. It could display a progress
            window, for example (hint, hint).
        */
        void accept();
    };

}

#endif
