/* -*- C++ -*-
   This file implements the detailed print style.

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

#include <kdebug.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kcolorbutton.h>
#include <klocale.h>
#include <kprinter.h>
#include <kfontcombo.h>
#include <kglobalsettings.h>
#include <knuminput.h>
#include <kdialog.h>
#include <kconfig.h>
#include <qpainter.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qpaintdevicemetrics.h>
#include "detailledstyle.h"
#include "printingwizard.h"
#include "printstyle.h"
#include "printprogress.h"

// ----- the wizard pages:
#include "ds_appearance.h"

namespace KABPrinting {

    const char* ConfigSectionName="DetailedPrintStyle";
    const char* UseKDEFonts="UseKDEFonts";
    const char* HeaderFont="HeaderFont";
    const char* HeaderFontSize="HeaderFontSize";
    const char* HeadlinesFont="HeadlineFont";
    const char* HeadlinesFontSize="HeadlineFontSize";
    const char* BodyFont="BodyFont";
    const char* BodyFontSize="BodyFontSize";
    const char* DetailsFont="DetailsFont";
    const char* DetailsFontSize="DetailsFontSize";
    const char* FixedFont="FixedFont";
    const char* FixedFontSize="FixedFontSize";
    const char* ColoredContactHeaders="UseColoredContactHeaders";
    const char* ContactHeaderForeColor="ContactHeaderForeColor";
    const char* ContactHeaderBGColor="ContactHeaderBGColor";


    DetailledPrintStyle::DetailledPrintStyle(PrintingWizard* parent,
                                             const char* name)
        : PrintStyle(parent, name),
          mPageAppearance(new AppearancePage(parent, "AppearancePage")),
          mEPntr(0),
          mPrintProgress(0)
    {
        KConfig *config;
        QFont font;
        bool kdeFonts;
        QFont standard=KGlobalSettings::generalFont();
        QFont fixed=KGlobalSettings::fixedFont();

        setPreview( "detailed-style.png" );

        addPage( mPageAppearance, i18n("Appearance") );
        // ----- set some values in the pages:
        // ----- now try to load previous settings from the
        // configuration file and set the UI items accordingly:
        config=kapp->config();
        config->setGroup(ConfigSectionName);
        //       use KDE fonts box:
        kdeFonts=config->readBoolEntry(UseKDEFonts, true);
        mPageAppearance->cbStandardFonts->setChecked(kdeFonts);
        //       header font:
        font=config->readFontEntry(HeaderFont, &standard);
        mPageAppearance->kfcHeaderFont->setCurrentFont(font.family());
        mPageAppearance->kisbHeaderFontSize->setValue(font.pointSize());
        font=config->readFontEntry(HeadlinesFont, &standard);
        mPageAppearance->kfcHeadlineFont->setCurrentFont(font.family());
        mPageAppearance->kisbHeadlineFontSize->setValue(font.pointSize());
        font=config->readFontEntry(BodyFont, &standard);
        mPageAppearance->kfcBodyFont->setCurrentFont(font.family());
        mPageAppearance->kisbBodyFontSize->setValue(font.pointSize());
        font=config->readFontEntry(DetailsFont, &standard);
        mPageAppearance->kfcDetailsFont->setCurrentFont(font.family());
        mPageAppearance->kisbDetailsFontSize->setValue(font.pointSize());
        font=config->readFontEntry(FixedFont, &fixed);
        mPageAppearance->kfcFixedFont->setCurrentFont(font.family());
        mPageAppearance->kisbFixedFontSize->setValue(font.pointSize());
        // ----- contact header design:
        mPageAppearance->cbBackgroundColor
            ->setChecked(config->readBoolEntry(ColoredContactHeaders, true));
        mPageAppearance->kcbHeaderBGColor
            ->setColor(config->readColorEntry(ContactHeaderForeColor, &Qt::black));
        mPageAppearance->kcbHeaderTextColor
            ->setColor(config->readColorEntry(ContactHeaderBGColor, &Qt::white));
        // ----- work around Qt Designer's fixed widget spacing:
        mPageAppearance->layout()->setMargin(KDialog::marginHint());
        mPageAppearance->layout()->setSpacing(KDialog::spacingHint());
        // ----- enable finish in our page:
    }

    DetailledPrintStyle::~DetailledPrintStyle()
    {
        if( mEPntr != 0 )
          delete mEPntr;
        mEPntr = 0;
    }

    void DetailledPrintStyle::print(QStringList contacts, PrintProgress *progress)
    {
        mPrintProgress=progress;
        progress->addMessage(i18n("Setting up fonts and colors"));
        progress->setProgress(0);
        bool useKDEFonts;
        KConfig *config;
        QFont font;
        QColor foreColor=Qt::black;
        QColor headerColor=Qt::white;
        bool useHeaderColor=true;
        QColor backColor=Qt::black;
        bool useBGColor;
        // save, always available defaults:
        QFont header=QFont("Helvetica", 12, QFont::Normal);
        QFont headlines=QFont("Helvetica", 12, QFont::Normal, true);
        QFont body=QFont("Helvetica", 12, QFont::Normal);
        QFont fixed=QFont("Courier", 12, QFont::Normal);
        QFont comment=QFont("Helvetica", 10, QFont::Normal);
        // ----- store the configuration settings:
        config=kapp->config();
        config->setGroup(ConfigSectionName);
        useKDEFonts=mPageAppearance->cbStandardFonts->isChecked();
        config->writeEntry(UseKDEFonts, useKDEFonts);
        // ----- read the font and color selections from the wizard pages:
        useBGColor=mPageAppearance->cbBackgroundColor->isChecked();
        config->writeEntry(ColoredContactHeaders, useBGColor);
        if(useBGColor)
        { // use colored contact headers, otherwise use plain black and white):
            headerColor=mPageAppearance->kcbHeaderTextColor->color();
            backColor=mPageAppearance->kcbHeaderBGColor->color();
            config->writeEntry(ContactHeaderForeColor, headerColor);
            config->writeEntry(ContactHeaderBGColor, backColor);
        }
        if(mPageAppearance->cbStandardFonts->isChecked())
        {
            QFont standard=KGlobalSettings::generalFont();
            header=standard;
            headlines=standard;
            body=standard;
            fixed=KGlobalSettings::fixedFont();
            comment=standard;
        } else {
            // ----- get header font settings and save for later
            // sessions:
            header.setFamily(mPageAppearance->kfcHeaderFont->currentText());
            header.setPointSize(mPageAppearance->kisbHeaderFontSize->value());
            config->writeEntry(HeaderFont, header);
            // ----- headlines:
            headlines.setFamily(mPageAppearance->kfcHeadlineFont->currentText());
            headlines.setPointSize(mPageAppearance->kisbHeadlineFontSize->value());
            config->writeEntry(HeadlinesFont, headlines);
            // ----- body:
            body.setFamily(mPageAppearance->kfcBodyFont->currentText());
            body.setPointSize(mPageAppearance->kisbBodyFontSize->value());
            config->writeEntry(BodyFont, body);
            // ----- details:
            comment.setFamily(mPageAppearance->kfcDetailsFont->currentText());
            comment.setPointSize(mPageAppearance->kisbDetailsFontSize->value());
            config->writeEntry(DetailsFont, comment);
            // ----- fixed:
            fixed.setFamily(mPageAppearance->kfcFixedFont->currentText());
            fixed.setPointSize(mPageAppearance->kisbFixedFontSize->value());
            config->writeEntry(FixedFont, fixed);
        }
        kdDebug() << "DetailledPrintStyle::print: printing using" << endl
                  << "    header:   " << header.family() << "("
                  << header.pointSize() << ")" << endl
                  << "    headline: " << headlines.family() << "("
                  << headlines.pointSize() << ")" << endl
                  << "    body:     " << body.family() << "("
                  << body.pointSize() << ")" << endl
                  << "    fixed:    " << fixed.family() << "("
                  << fixed.pointSize() << ")" << endl
                  << "    comment:  " << comment.family() << "("
                  << comment.pointSize() << ")" << endl;

        mEPntr=new KABEntryPainter(foreColor, headerColor, useHeaderColor,
                                   backColor,
                                   header, headlines, body, fixed, comment);
        KPrinter *printer=wizard()->printer();
        QPainter painter;
        // ----- variables used to define MINIMAL MARGINS entered by the user:
        progress->addMessage(i18n("Setting up margins and spacing"));
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
        progress->addMessage(i18n("Printing"));
        printEntries(contacts, printer, &painter,
                     QRect(0, 0, metrics.width(), metrics.height()));
        progress->addMessage(i18n("Done"));
        painter.end();
        config->sync();
    }

    bool DetailledPrintStyle::printEntries(const QStringList& contacts,
                                           KPrinter *printer,
                                           QPainter *painter,
                                           const QRect& window)
    {
        KABC::Addressee addressee;
        QStringList::ConstIterator it;
        QRect brect;
        int ypos=0, count=0;
        // -----
        for(it=contacts.begin(); it!=contacts.end(); ++it)
        {
            addressee=wizard()->document()->findByUid(*it);
            if(!addressee.isEmpty())
            { // print it:
                kdDebug() << "DetailledPrintStyle::printEntries: printing addressee "
                          << addressee.realName() << endl;
                // ----- do a faked print to get the bounding rect:
                if(!mEPntr->printEntry(addressee, window, painter, ypos, true, &brect))
                { // it does not fit on the page beginning at ypos:
                    printer->newPage();
                    // WORK_TO_DO: this assumes the entry fits on the whole page
                    // (dunno how to fix this without being illogical)
                    ypos=0;
                }
                mEPntr->printEntry(addressee, window, painter, ypos, false, &brect);
                ypos+=brect.height();
            } else {
                kdDebug() << "DetailledPrintStyle::printEntries: strange, addressee "
                          << "with UID " << *it << " not available." << endl;
            }
            // ----- set progress:
            mPrintProgress->setProgress((count++*100)/contacts.count());
        }
        mPrintProgress->setProgress(100);
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
        return new DetailledPrintStyle( mParent, mName );
    }

    QString DetailledPrintStyleFactory::description()
    {
        return i18n("Detailed Style");
    }

}

#include "detailledstyle.moc"
