#include <qpainter.h>
#include <qpaintdevicemetrics.h>

#include <kapplication.h>
#include <kprinter.h>
#include <klocale.h>
#include <kdebug.h>
#include <kprogress.h>
#include <kabc/addressee.h>

#include "printingwizard.h"
#include "mikesstyle.h"

namespace KABPrinting
{

    const int mFieldSpacingHint=2;

    MikesStyle::MikesStyle(PrintingWizard* parent, const char* name)
        : PrintStyle(parent, name)
    {
    }

    MikesStyle::~MikesStyle()
    {
    }

    void MikesStyle::print(QStringList printUids)
    {
        QFont mFont;
        QFont mBoldFont;
        QPainter p;
        p.begin(wiz->printer());
        int yPos = 0;
        int spacingHint = 10;
        // Now do the actual printing
        KProgressDialog pDialog
            (wiz, 0, i18n("Printing Progress"),
             i18n("Please wait while the contacts are prepared "
                  "for the printer."));
        pDialog.setAutoClose(true);
        pDialog.setAllowCancel(false);
        KProgress *progressBar = pDialog.progressBar();
        progressBar->setRange(1, printUids.count());
        pDialog.show();

        mFont = p.font();
        mBoldFont = p.font();
        mBoldFont.setBold(true);
        QFontMetrics fm(mFont);
        QPaintDeviceMetrics metrics(p.device());

        int height = 0;
        KABC::Addressee a;
        int index = 1;
        QStringList::ConstIterator iter;

        kdDebug() << "MikesStyle::print: now printing." << endl;


        for (iter = printUids.begin(); iter != printUids.end(); ++iter)
        {
            // Update the progress bar
            progressBar->setValue(index++);
            kapp->processEvents(); // Mirko: do I like this :-) ?

            // find the addressee
            a = wiz->document()->findByUid(*iter);

            // Get the total height so we know if it will fit on the
            // current page
            height = calcHeight(a, mFont, mBoldFont);
            if ((yPos + spacingHint + height)
                > (metrics.height()-fm.height()-5))
            {
                p.save();
                p.translate(0, metrics.height()-fm.height()-5);
                paintTagLine(p, mFont);
                p.restore();

                wiz->printer()->newPage();
                yPos = 0;
            }

            // Move the painter to the proper position and then paint the
            // addressee
            yPos += spacingHint;
            p.save();
            p.translate(0, yPos);
            doPaint(p, a, height, mFont, mBoldFont);
            p.restore();
            yPos += height;
        }
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
        kdDebug() << "MikesStyle::doPaint -->" << endl;

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

        KABC::Field::List fields = wiz->document()->fields();
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

        kdDebug() << "MikesStyle::doPaint <--" << endl;
    }

    void MikesStyle::paintTagLine(QPainter &p, const QFont& font)
    {
        kdDebug() << "MikesStyle::paintTagLine: -->" << endl;

        QFontMetrics fm(font);

        QString text =
            i18n("Printed on %1 by KAddressBook (http://www.kde.org)")
            .arg(QDateTime::currentDateTime().toString());

        p.setPen(Qt::black);
        p.drawText(0, fm.height(), text);

        kdDebug() << "MikesStyle::paintTagLine: <--" << endl;
    }

    int MikesStyle::calcHeight(const KABC::Addressee &a,
                               const QFont& font, const QFont& bFont)
    {
        kdDebug() << "MikesStyle::calcHeight: -->" << endl;

        QFontMetrics fm(font);
        QFontMetrics bfm(bFont);

        int height = 0;

        // get the fields
        KABC::Field::List fieldList = wiz->document()->fields();
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

        kdDebug() << "MikesStyle::calcHeight: <--" << endl;
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
        return new MikesStyle(parent, name);
    }

    QString MikesStyleFactory::description()
    {
        return i18n("Mike's Printing Style");
    }

}

#include "mikesstyle.moc"

