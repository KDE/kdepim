/* -*- C++ -*-
   This file implements the printing wizard.

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

#include <qradiobutton.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <kdebug.h>
#include <kprinter.h>
#include <klocale.h>
#include <kdialog.h>
#include <kdialogbase.h>
#include <kapplication.h>

#include "printingwizard.h"
#include "printstyle.h"
#include "printprogress.h"
#include "detailledstyle.h"
#include "mikesstyle.h"

namespace KABPrinting {

    PrintingWizardImpl::PrintingWizardImpl(KPrinter *printer,
                                           KABC::AddressBook* doc,
                                           const QStringList& selection,
                                           QWidget *parent,
                                           const char* name)
        : PrintingWizard(printer, doc, selection, parent, name),
          style(0)
    {
        mBasicPage=new BasicPage(this);
        mBasicPage->rbSelection->setEnabled(!selection.isEmpty());
        connect(mBasicPage->cbStyle, SIGNAL(activated(int)),
                SLOT(slotStyleSelected(int)));
        insertPage(mBasicPage, i18n("General"), -1);
        setAppropriate(mBasicPage, true);
        registerStyles();
        if(mBasicPage->cbStyle->count()>0)
        {
            slotStyleSelected(0);
        }
    }

    PrintingWizardImpl::~PrintingWizardImpl()
    {
    }

    void PrintingWizardImpl::accept()
    { // progress display needs to be implemented here:
        print();
        close();
    }

    void PrintingWizardImpl::registerStyles()
    {
        styleFactories.append(new DetailledPrintStyleFactory(this));
        styleFactories.append(new MikesStyleFactory(this));

        mBasicPage->cbStyle->clear();
        for(unsigned int i=0; i<styleFactories.count(); ++i)
        {
            mBasicPage->cbStyle->insertItem(styleFactories.at(i)->description());
        }
    }

    void PrintingWizardImpl::slotStyleSelected( int index )
    {
      if ( index < 0 || index >= styleFactories.count() )
        return;

      setFinishEnabled( mBasicPage, false );

      if ( style )
        style->hidePages();

      if ( mStyleList.at( index ) != 0 )
        style = mStyleList.at( index );
      else {
        PrintStyleFactory *factory = styleFactories.at( index );
        kdDebug() << "PrintingWizardImpl::slotStyleSelected: "
                  << "creating print style "
                  << factory->description() << endl;
        style = factory->create();
        mStyleList.insert( index, style );
      }

      style->showPages();

      const QPixmap& preview = style->preview();
      mBasicPage->plPreview->setPixmap( preview );
      if ( preview.isNull() )
        mBasicPage->plPreview->setText( i18n( "(No preview available.)" ) );

      setFinishEnabled( page( pageCount() - 1 ), true );
    }

    KABC::AddressBook* PrintingWizardImpl::document()
    {
        return mDocument;
    }

    KPrinter* PrintingWizardImpl::printer()
    {
        return mPrinter;
    }

    // WORK_TO_DO: select contacts for printing
    void PrintingWizardImpl::print()
    {
        // ----- create and show print progress widget:
        PrintProgress *progress=new PrintProgress(this);
        insertPage(progress, i18n("Print Progress"), -1);
        showPage(progress);
        kapp->processEvents();
        // ----- prepare list of contacts to print:
        QStringList contacts;
        if(style!=0)
        {
            if(mBasicPage->rbSelection->isChecked())
            {
                contacts=mSelection;
            } else {
                // create a string list of all entries:
                KABC::AddressBook::Iterator iter;
                for(iter=document()->begin(); iter!=document()->end(); ++iter)
                {
                    contacts << (*iter).uid();
                }
            }
        }
        kdDebug() << "PrintingWizardImpl::print: printing "
                  << contacts.count() << " contacts." << endl;
        // ... print:
        setBackEnabled(progress, false);
        cancelButton()->setEnabled(false);
        style->print(contacts, progress);
        // ----- done - all GUI elements will disappear
    }

}

#include "printingwizard.moc"
