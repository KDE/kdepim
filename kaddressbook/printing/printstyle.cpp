/* -*- C++ -*-
   This file implements the abstract print style base class.

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

#include <kstandarddirs.h>
#include <kdebug.h>

#include "printstyle.h"
#include "printingwizard.h"

namespace KABPrinting {

    PrintStyle::PrintStyle(PrintingWizard* parent, const char* name)
        : QObject(parent, name),
          mWizard(parent)
    {
    }

    PrintStyle::~PrintStyle()
    {
    }

    const QPixmap& PrintStyle::preview()
    { // this is Null pixmap as long as nothing is assigned to it:
        return mPreview;
    }

    void PrintStyle::setPreview(const QPixmap& image)
    {
        mPreview=image; // we do not check for Null images etc
    }

    bool PrintStyle::setPreview(const QString& filename)
    {
        // ----- locate the preview image and set it:
        QPixmap preview;
        QString file=(QString)"printing/";
        file.append(filename);
        QString path=locate("appdata", file);
        if(path.isNull())
        {
            kdDebug() << "PrintStyle::setPreview: preview not locatable." << endl;
            return false;
        } else {
            if(preview.load(path))
            {
                setPreview(preview);
                return true;
            } else {
                kdDebug() << "PrintStyle::setPreview: preview at "
                          << path << " cannot be loaded."
                          << endl;
                return false;
            }
        }


    }

    PrintingWizard *PrintStyle::wizard()
    {
        return mWizard;
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
