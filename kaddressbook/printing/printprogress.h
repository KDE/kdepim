/* -*- C++ -*-
   This file declares the print progress widget.

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

#ifndef PRINTPROGRESS_H
#define PRINTPROGRESS_H

#include <qstringlist.h>
#include "printprogress_base.h"

namespace KABPrinting {

    /** This defines a simple widget to display print progress
        information. It is provided to all print styles during a print
        process. It displays messages and a a progress bar.
    */

    class PrintProgress : public PrintProgressBase
    {
        Q_OBJECT
    public:
        PrintProgress(QWidget *parent);
        ~PrintProgress();
        /** Add a message to the message log. Give the user something
            to admire :-)
        */
        void addMessage(const QString &);
        /** Set the progress to a certain amount. Steps are from Zero
            to 100.
        */
        void setProgress(int);
    private:
        QStringList messages;
    };

}

#endif
