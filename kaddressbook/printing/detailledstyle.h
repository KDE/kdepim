/* -*- C++ -*-
   This file declares the detailed print style.

   the KDE addressbook

   $ Author: Mirko Boehm $
   $ Copyright: (C) 1996-2002, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Revision$
*/

#ifndef DETAILLEDSTYLE_H
#define DETAILLEDSTYLE_H

#include <kabc/addressee.h>

#include "printstyle.h"
#include "kabentrypainter.h"

class AppearancePage;

namespace KABPrinting {

    class DetailledPrintStyle : public PrintStyle
    {
        Q_OBJECT
    public:
        DetailledPrintStyle(PrintingWizard* parent, const char* name=0);
        ~DetailledPrintStyle();
        void print(QStringList contacts, PrintProgress*);
    protected:
        bool printEntries(const QStringList& contacts,
                          KPrinter *printer,
                          QPainter *painter,
                          const QRect& window);
        bool printEntry(const KABC::Addressee& contact,
                        const QRect& window,
                        QPainter *painter,
                        int top, bool fake, QRect *brect);
    private:
        AppearancePage *mPageAppearance;
        KABEntryPainter *mEPntr;
        PrintProgress *mPrintProgress;
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
