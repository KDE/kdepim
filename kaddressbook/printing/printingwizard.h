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
        PrintStyle *style;
        /** The general page. */
        BasicPage *mBasicPage;
        QVBoxLayout* pageLayout;
    };

}

#endif
