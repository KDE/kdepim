#include <kdebug.h>
#include <klocale.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include "detailledstyle.h"
#include "printingwizard.h"

namespace KABPrinting {

    DetailledPrintStyle::DetailledPrintStyle(PrintingWizard* parent,
                                             const char* name)
        : PrintStyle(parent, name)
    {
    }

    DetailledPrintStyle::~DetailledPrintStyle()
    {
    }

    void DetailledPrintStyle::print(QStringList)
    {
        kdDebug() << "DetailledPrintStyle::print" << endl;
    }

    DetailledPrintStyleFactory::DetailledPrintStyleFactory(
        PrintingWizard* parent,
        const char* name)
        : PrintStyleFactory(parent, name)
    {
    }


    PrintStyle *DetailledPrintStyleFactory::create()
    {
        return new DetailledPrintStyle(parent, name);
    }

    QString DetailledPrintStyleFactory::description()
    {
        return i18n("Detailled Style");
    }
}

#include "detailledstyle.moc"
