#include <kdebug.h>
#include <klocale.h>
#include <kprinter.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include "detailledstyle.h"
#include "printingwizard.h"

namespace KABPrinting {

    DetailledPrintStyle::DetailledPrintStyle(PrintingWizard* parent,
                                             const char* name)
        : PrintStyle(parent, name),
          ePntr(Qt::black, Qt::white, true, Qt::black)
    {
    }

    DetailledPrintStyle::~DetailledPrintStyle()
    {
    }

    void DetailledPrintStyle::print(QStringList contacts)
    {
        KPrinter *printer=wiz->printer();
        QPainter painter;
        // ----- variables used to define MINIMAL MARGINS entered by the user:
        int marginTop=0,
           marginLeft=64, // to allow stapling, need refinement with two-side prints
          marginRight=0,
         marginBottom=0;
        register int left, top, width, height;
        // ----- we expect the printer to be set up (it is, before the wizard is started):
        painter.begin(printer);
        printer->setFullPage(true); // use whole page
        QPaintDeviceMetrics metrics(printer);
        kdDebug() << "DetailledPrintStyle::print: printing on a "
                  << metrics.width() << "x" << metrics.height()
                  << " size area," << endl << "   "
                  << "margins are "
                  << printer->margins().width() << " (left/right) and "
                  << printer->margins().height() << " (top/bottom)." << endl;
        left=QMAX(printer->margins().width(), marginLeft); // left margin
        top=QMAX(printer->margins().height(), marginTop); // top margin
        width=metrics.width()-left
              -QMAX(printer->margins().width(), marginRight); // page width
        height=metrics.height()-top
               -QMAX(printer->margins().height(), marginBottom); // page height
        // ----- now do the printing:
        // this prepares for, like, two-up etc:
        painter.setViewport(left, top, width, height);
        printEntries(contacts, printer, &painter,
                     QRect(0, 0, metrics.width(), metrics.height()));
        painter.end();
    }

    bool DetailledPrintStyle::printEntries(const QStringList& contacts,
                                           KPrinter *printer,
                                           QPainter *painter,
                                           const QRect& window)
    {
        KABC::Addressee addressee;
        QStringList::ConstIterator it;
        QRect brect;
        int ypos=0;
        // -----
        for(it=contacts.begin(); it!=contacts.end(); ++it)
        {
            addressee=wiz->document()->findByUid(*it);
            if(!addressee.isEmpty())
            { // print it:
                kdDebug() << "DetailledPrintStyle::printEntries: printing addressee "
                          << addressee.realName() << endl;
                // ----- do a faked print to get the bounding rect:
                if(!ePntr.printEntry(addressee, window, painter, ypos, true, &brect))
                { // it does not fit on the page beginning at ypos:
                    printer->newPage();
                    // WORK_TO_DO: this assumes the entry fits on the whole page
                    // (dunno how to fix this without being illogical)
                    ypos=0;
                }
                ePntr.printEntry(addressee, window, painter, ypos, false, &brect);
                ypos+=brect.height();
            } else {
                kdDebug() << "DetailledPrintStyle::printEntries: strange, addressee "
                          << "with UID " << *it << " not available." << endl;
            }
        }
        return true;
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
