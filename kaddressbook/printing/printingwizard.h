#ifndef PRINTINGWIZARD_H
#define PRINTINGWIZARD_H

#include <qlist.h>
#include <qstringlist.h>

#include "printingwizard_base.h"
#include "kabc/addressbook.h"
#include "printstyle.h"

class KPrinter;

namespace KABPrinting {

    /** The PrintingWizard combines pages common for all print styles
        and those provided by the respective style.
    */
    class PrintingWizard : public PrintingWizardBase
    {
        Q_OBJECT
    public:
        /** Construct a printing wizard. Give the document
        (addressbook instance) to print.
        */
        PrintingWizard(KPrinter *printer,
                       KABC::AddressBook* doc,
                       const QStringList& selection,
                       QWidget *parent=0, const char *name=0);
        ~PrintingWizard();
        /** Modify this method to add a new PrintStyle.
         */
        void registerStyles();
        /** Perform the actual printing. */
        void print();
        /** Retrieve the document object. */
        KABC::AddressBook *document();
        /** Retrieve the printer to be used. */
        KPrinter* printer();
    protected:
        /** A print style has been selected. The argument is the index
            in the cbStyle combo and in styles.
        */
        void slotStyleSelected(int);
        QPtrList<PrintStyleFactory> styleFactories;
        PrintStyle *style;
        KABC::AddressBook *mDocument;
        QStringList selection;
        /** The printer is handed over at construction time and needs
            to be already set up.
            The printer is handed down to the PrintStyle object that
            actually prints.
        */
        KPrinter *mPrinter;
    };

}

#endif
