#include <qradiobutton.h>
#include <qcombobox.h>

#include <kdebug.h>
#include <kprinter.h>

#include "printingwizard.h"
#include "printstyle.h"
#include "detailledstyle.h"
#include "mikesstyle.h"

namespace KABPrinting {

    PrintingWizard::PrintingWizard(KPrinter *printer_,
                                   KABC::AddressBook* doc,
                                   const QStringList& selection_,
                                   QWidget *parent,
                                   const char* name)
        : PrintingWizardBase(parent, name),
          style(0),
          mDocument(doc),
          selection(selection_),
          mPrinter(printer_)
    {
        rbSelection->setEnabled(!selection.isEmpty());
        setAppropriate(General, true);
        registerStyles();
        if(cbStyle->count()>0)
        {
            slotStyleSelected(0);
        }
    }

    PrintingWizard::~PrintingWizard()
    {
    }

    void PrintingWizard::registerStyles()
    {
        styleFactories.append(new DetailledPrintStyleFactory(this));
        styleFactories.append(new MikesStyleFactory(this));

        cbStyle->clear();
        for(unsigned int i=0; i<styleFactories.count(); ++i)
        {
            cbStyle->insertItem(styleFactories.at(i)->description());
        }
    }

    void PrintingWizard::slotStyleSelected(int index)
    {
        if(index>=0 && index<cbStyle->count())
        {
            if(style!=0)
            {
                delete style;
                style=0;
            }
            PrintStyleFactory *factory=styleFactories.at(index);
            kdDebug() << "PrintingWizard::slotStyleSelected: "
                      << "creating print style "
                      << factory->description() << endl;
            style=factory->create();
        }
        setFinishEnabled(General, style!=0);
    }

    KABC::AddressBook* PrintingWizard::document()
    {
        return mDocument;
    }

    KPrinter* PrintingWizard::printer()
    {
        return mPrinter;
    }

    // WORK_TO_DO: select contacts for printing
    void PrintingWizard::print()
    {
        if(style!=0)
        {
            if(rbCurrentContact->isChecked())
            {
                // ... print current entry
            } else {
                if(rbSelection->isChecked())
                {
                    style->print(selection);
                } else {
                    // ... print all contacts
                    style->print(QStringList());
                }
            }
        }
    }

}

#include "printingwizard.moc"
