#ifndef DETAILLEDSTYLE_H
#define DETAILLEDSTYLE_H

#include "printstyle.h"

namespace KABPrinting {

    class DetailledPrintStyle : public PrintStyle
    {
        Q_OBJECT
    public:
        DetailledPrintStyle(PrintingWizard* parent, const char* name=0);
        ~DetailledPrintStyle();
        void print(QStringList contacts);
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
