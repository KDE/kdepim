#include "printstyle.h"
#include "printingwizard.h"

namespace KABPrinting {

    PrintStyle::PrintStyle(PrintingWizard* parent, const char* name)
        : QObject(parent, name),
          wiz(parent)
    {
    }

    PrintStyle::~PrintStyle()
    {
    }

    PrintStyleFactory::PrintStyleFactory(PrintingWizard* parent_,
                                         const char* name_)
        : parent(parent_),
          name(name_)
    {
    }
}

#include "printstyle.moc"
