/* -*- C++ -*-
   This file declares Mike Pilone's printing style.

   the KDE addressbook

   $ Author: Mirko Boehm $
   (C) 2002, Mike Pilone
   $ Copyright: (C) 1996-2002, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Revision$
*/

#ifndef MIKESSTYLE_H
#define MIKESSTYLE_H

#include <qfont.h>

#include "printstyle.h"

namespace KABPrinting {

    class PrintProgress;

    class MikesStyle : public PrintStyle
    {
        Q_OBJECT
    public:
        MikesStyle(PrintingWizard* parent, const char* name);
        ~MikesStyle();
        void print(KABC::Addressee::List&, PrintProgress*);
    protected:
        void doPaint(QPainter &painter, const KABC::Addressee &a,
                     int maxHeight,
                     const QFont& font, const QFont& bFont);
        int calcHeight(const KABC::Addressee &a,
                       const QFont& font, const QFont& bFont);
        void paintTagLine(QPainter &p, const QFont& font);
        QString trimString(const QString &text, int width,
                           QFontMetrics &fm);
    };

    class MikesStyleFactory : public PrintStyleFactory
    {
    public:
        MikesStyleFactory(PrintingWizard* parent_,
                                   const char* name_=0);
        PrintStyle *create();
        QString description();
    };

}

#endif
