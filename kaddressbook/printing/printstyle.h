#ifndef PRINTSTYLE_H
#define PRINTSTYLE_H

#include <qwidget.h>
#include <qstringlist.h>

namespace KABPrinting {

    class PrintingWizard;

    class PrintStyle : public QObject
    {
        Q_OBJECT
    public:
        PrintStyle(PrintingWizard* parent, const char* name=0);
        virtual ~PrintStyle();
        /** Reimplement this method to actually print. */
        virtual void print(QStringList contacts)=0;
    protected:
        PrintingWizard *wiz;
    };


    /** The factories are used to have all object of the respective
        print style created in one place.
        This will maybe be changed to a template because of its simple
        nature :-)
    */
    class PrintStyleFactory
    {
    public:
        PrintStyleFactory(PrintingWizard* parent,
                          const char* name=0);
        virtual PrintStyle *create()=0;
        /** Overload this method to provide a one-liner description
            for your print style. */
        virtual QString description()=0;
    protected:
        PrintingWizard* parent;
        const char* name;
    };
}

#endif
