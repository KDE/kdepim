#ifndef DETAILLEDSTYLE_H
#define DETAILLEDSTYLE_H

#include <kabc/addressee.h>
#include "printstyle.h"
// this is bad style, but we do not have another common library by now:
#include "../kabentrypainter.h"

namespace KABPrinting {

    class DetailledPrintStyle : public PrintStyle
    {
        Q_OBJECT
    public:
        DetailledPrintStyle(PrintingWizard* parent, const char* name=0);
        ~DetailledPrintStyle();
        void print(QStringList contacts);
    protected:
        bool printEntries(const QStringList& contacts,
                          KPrinter *printer,
                          QPainter *painter,
                          const QRect& window);
        bool printEntry(const KABC::Addressee& contact,
                        const QRect& window,
                        QPainter *painter,
                        int top, bool fake, QRect *brect);
        KABEntryPainter ePntr;
    };

    class DetailledPrintStyleFactory : public PrintStyleFactory
    {
    public:
        DetailledPrintStyleFactory(PrintingWizard* parent_,
                                   const char* name_=0);
        PrintStyle *create();
        QString description();
    };

}

#endif
