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

    const QPixmap& PrintStyle::preview()
    { // this is Null pixmap as long as nothing is assigned to it:
        return mPreview;
    }

    PrintStyleFactory::PrintStyleFactory(PrintingWizard* parent_,
                                         const char* name_)
        : parent(parent_),
          name(name_)
    {
    }

    PrintStyleFactory::~PrintStyleFactory()
    {
    }

}

#include "printstyle.moc"
