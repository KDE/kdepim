/* -*- C++ -*-
   This file implements Mike's printing style.

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

#include <qpainter.h>
#include <qpaintdevicemetrics.h>

#include <kapplication.h>
#include <kprinter.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kprogress.h>
#include <kabc/addressee.h>

#include "printingwizard.h"
#include "printstyle.h"
#include "printprogress.h"
#include "mikesstyle.h"

namespace KABPrinting
{

    const int mFieldSpacingHint=2;

    MikesStyle::MikesStyle(PrintingWizard* parent, const char* name)
        : PrintStyle(parent, name)
    {
      setPreview("mike-style.png");
    }

    MikesStyle::~MikesStyle()
    {
    }

    void MikesStyle::print( KABC::Addressee::List &contacts, PrintProgress *progress)
    {
        QFont mFont;
        QFont mBoldFont;
        QPainter p;
        p.begin(wizard()->printer());
        int yPos = 0, count=0;
        int spacingHint = 10;
        // Now do the actual printing
        mFont = p.font();
        mBoldFont = p.font();
        mBoldFont.setBold(true);
        QFontMetrics fm(mFont);
        QPaintDeviceMetrics metrics(p.device());

        int height = 0;
        KABC::Addressee::List::Iterator it;

        progress->addMessage(i18n("Preparing"));
        progress->addMessage(i18n("Printing"));

        for ( it = contacts.begin(); it != contacts.end(); ++it )
        {
            progress->setProgress((count++*100)/contacts.count());
            kapp->processEvents();

            // Get the total height so we know if it will fit on the
            // current page
            height = calcHeight((*it), mFont, mBoldFont);
            if ((yPos + spacingHint + height)
                > (metrics.height()-fm.height()-5))
            {
                p.save();
                p.translate(0, metrics.height()-fm.height()-5);
                paintTagLine(p, mFont);
                p.restore();

                wizard()->printer()->newPage();
                yPos = 0;
            }

            // Move the painter to the proper position and then paint the
            // addressee
            yPos += spacingHint;
            p.save();
            p.translate(0, yPos);
            doPaint(p, (*it), height, mFont, mBoldFont);
            p.restore();
            yPos += height;
            // ----- set progress bar:
            // WORK_TO_DO: port to common progress display scheme
        }
        progress->addMessage(i18n("Done"));
        // print the tag line on the last page
        p.save();
        p.translate(0, metrics.height()-fm.height()-5);
        paintTagLine(p, mFont);
        p.restore();

        // send to the printer
        p.end();
    }

    QString MikesStyle::trimString(const QString &text, int width,
                                   QFontMetrics &fm)
    {
        if (fm.width(text) <= width)
            return text;

        QString dots = "...";
        int dotWidth = fm.width(dots);
        QString trimmed;
        int charNum = 0;

        while (fm.width(trimmed) + dotWidth < width)
        {
            trimmed += text[charNum];
            charNum++;
        }

        // Now trim the last char, since it put the width over the top
        trimmed = trimmed.left(trimmed.length()-1);
        trimmed += dots;

        return trimmed;
    }

    void MikesStyle::doPaint(QPainter &painter, const KABC::Addressee &a,
                             int maxHeight,
                             const QFont& font, const QFont& bFont)
    {
        QFontMetrics fm(font);
        QFontMetrics bfm(bFont);
        QPaintDeviceMetrics metrics(painter.device());
        int margin = 10;
        int width = metrics.width() - 10;
        int xPos = 5;
        int yPos = 0;
        QBrush brush(Qt::lightGray);

        painter.setPen(Qt::black);
        painter.drawRect(xPos, yPos, width, maxHeight);

        // The header
        painter.fillRect(xPos+1, yPos+1, width-2,
                         bfm.height() + 2*mFieldSpacingHint - 2, brush);
        painter.setFont(bFont);
        xPos += mFieldSpacingHint;
        painter.drawText(xPos, yPos+bfm.height(),
                         a.formattedName());

        yPos += bfm.height() + 2*mFieldSpacingHint;
        xPos = margin;

        // now the fields, in two halves
        painter.setFont(font);

        KABC::Field::List fields = wizard()->document()->fields();
        int numFields = fields.count();
        QString label;
        QString value;

        for (int i = 0; i < numFields/2; i++)
        {
            label = fields[i]->label();
            value = trimString(fields[i]->value(a), (width-10)/4, fm);

            yPos += fm.height();
            painter.drawText(xPos, yPos, label + ":");

            xPos += (width - (2 * margin))/4;
            painter.drawText(xPos, yPos, value);

            yPos += mFieldSpacingHint;
            xPos = margin;
        }

        yPos = bfm.height() + 2*mFieldSpacingHint;
        xPos = margin + width/2;
        for (int i = numFields/2; i < numFields; i++)
        {
            label = fields[i]->label();
            value = value = trimString(fields[i]->value(a), (width-10)/4, fm);

            yPos += fm.height();
            painter.drawText(xPos, yPos, label + ":");

            xPos += (width - (2 * margin))/4;
            painter.drawText(xPos, yPos, value);

            yPos += mFieldSpacingHint;
            xPos = margin + width/2;
        }

    }

    void MikesStyle::paintTagLine(QPainter &p, const QFont& font)
    {
        QFontMetrics fm(font);

        QString text =
            i18n("Printed on %1 by KAddressBook (http://www.kde.org)")
            .arg(KGlobal::locale()->formatDateTime(QDateTime::currentDateTime()));

        p.setPen(Qt::black);
        p.drawText(0, fm.height(), text);

    }

    int MikesStyle::calcHeight(const KABC::Addressee &a,
                               const QFont& font, const QFont& bFont)
    {

        QFontMetrics fm(font);
        QFontMetrics bfm(bFont);

        int height = 0;

        // get the fields
        KABC::Field::List fieldList = wizard()->document()->fields();
        int numFields = fieldList.count();
        int halfHeight = 0;

        // Determine which half of the fields is higher
        for (int i = 0; i < numFields/2; i++)
        {
            halfHeight +=
                fm.height() * (fieldList[i]->value(a).contains('\n') + 1);
        }
        height = halfHeight;

        // now the second half
        halfHeight = 0;
        for (int i = numFields/2; i < numFields; i++)
        {
            halfHeight +=
                fm.height() * (fieldList[i]->value(a).contains('\n') + 1);
        }

        height = QMAX(height, halfHeight);

        // Add the title and the spacing
        height += bfm.height() + ((numFields/2 + 3)*mFieldSpacingHint);

        return height;

    }

    // The factory class:

    MikesStyleFactory::MikesStyleFactory(PrintingWizard* parent,
                                         const char* name)
        : PrintStyleFactory(parent, name)
    {
    }

    PrintStyle *MikesStyleFactory::create()
    {
        return new MikesStyle( mParent, mName );
    }

    QString MikesStyleFactory::description()
    {
        return i18n("Mike's Printing Style");
    }

}

#include "mikesstyle.moc"

