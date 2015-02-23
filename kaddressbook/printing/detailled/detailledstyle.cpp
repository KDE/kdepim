/*
  This file is part of KAddressBook.
  Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>
                     2009 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "detailledstyle.h"

#include "printingwizard.h"
#include "printprogress.h"
#include "printstyle.h"
#include "ui_ds_appearance.h"

#include <KConfig>
#include <QDialog>
#include <KLocalizedString>
#include <KConfigGroup>
#include <QCheckBox>
#include <QPrinter>
#include <QTextDocument>
#include <KSharedConfig>
#include <QLocale>

using namespace KABPrinting;

const char *ConfigSectionName = "DetailedPrintStyle";
const char *ContactHeaderForeColor = "ContactHeaderForeColor";
const char *ContactHeaderBGColor = "ContactHeaderBGColor";

struct ContactBlock {
    typedef QList<ContactBlock> List;

    QString header;
    QStringList entries;
};

struct ColorSettings {
    QString headerTextColor;
    QString headerBackgroundColor;
};

QString contactsToHtml(const KContacts::Addressee::List &contacts, const ColorSettings &settings)
{
    QString content;

    content += QLatin1String("<html>\n");
    content += QLatin1String(" <head>\n");
    content += QLatin1String("  <style type=\"text/css\">\n");
    content += QLatin1String("    td.indented {\n");
    content += QLatin1String("      padding-left: 20px;\n");
    content += QLatin1String("      font-family: Fixed, monospace;\n");
    content += QLatin1String("    }\n");
    content += QLatin1String("  </style>\n");
    content += QLatin1String(" </head>\n");
    content += QLatin1String(" <body>\n");
    foreach (const KContacts::Addressee &contact, contacts) {
        QString name = contact.realName();
        if (!contact.title().isEmpty() || !contact.role().isEmpty()) {
            QStringList content;
            if (!contact.title().isEmpty()) {
                content << contact.title();
            }
            if (!contact.role().isEmpty()) {
                content << contact.role();
            }
            name += QStringLiteral(" (%1)").arg(content.join(QLatin1String(", ")));
        }

        const QString birthday = QLocale().toString(contact.birthday().date(), QLocale::ShortFormat);

        ContactBlock::List blocks;

        if (!contact.organization().isEmpty()) {
            ContactBlock block;
            block.header = i18n("Organization:");
            block.entries.append(contact.organization());

            blocks.append(block);
        }

        if (!contact.emails().isEmpty()) {
            ContactBlock block;
            block.header = (contact.emails().count() == 1 ?
                            i18n("Email address:") :
                            i18n("Email addresses:"));
            block.entries = contact.emails();

            blocks.append(block);
        }

        if (!contact.phoneNumbers().isEmpty()) {
            const KContacts::PhoneNumber::List numbers = contact.phoneNumbers();

            ContactBlock block;
            block.header = (numbers.count() == 1 ?
                            i18n("Telephone:") :
                            i18n("Telephones:"));

            foreach (const KContacts::PhoneNumber &number, numbers) {
                const QString line = number.typeLabel() + QLatin1String(": ") + number.number();
                block.entries.append(line);
            }

            blocks.append(block);
        }

        if (contact.url().isValid()) {
            ContactBlock block;
            block.header = i18n("Web page:");
            block.entries.append(contact.url().toDisplayString());

            blocks.append(block);
        }

        if (!contact.addresses().isEmpty()) {
            const KContacts::Address::List addresses = contact.addresses();

            foreach (const KContacts::Address &address, addresses) {
                ContactBlock block;

                switch (address.type()) {
                case KContacts::Address::Dom:
                    block.header = i18n("Domestic Address");
                    break;
                case KContacts::Address::Intl:
                    block.header = i18n("International Address");
                    break;
                case KContacts::Address::Postal:
                    block.header = i18n("Postal Address");
                    break;
                case KContacts::Address::Parcel:
                    block.header = i18n("Parcel Address");
                    break;
                case KContacts::Address::Home:
                    block.header = i18n("Home Address");
                    break;
                case KContacts::Address::Work:
                    block.header = i18n("Work Address");
                    break;
                case KContacts::Address::Pref:
                default:
                    block.header = i18n("Preferred Address");
                }
                block.header += QLatin1Char(':');

                block.entries = address.formattedAddress().split(QLatin1Char('\n'), QString::KeepEmptyParts);
                blocks.append(block);
            }
        }

        if (!contact.note().isEmpty()) {
            ContactBlock block;
            block.header = i18n("Notes:");
            block.entries = contact.note().split(QLatin1Char('\n'), QString::KeepEmptyParts);

            blocks.append(block);
        }

        // add header
        content += QLatin1String("  <table style=\"border-width: 0px; border-spacing: 0px; "
                                 "page-break-inside: avoid\" cellspacing=\"0\" cellpadding=\"0\" width=\"100%\">\n");
        content += QLatin1String("   <tr>\n");
        content += QLatin1String("    <td style=\"color: ") + settings.headerTextColor +
                   QLatin1String(";\" bgcolor=\"") + settings.headerBackgroundColor +
                   QLatin1String("\" style=\"padding-left: 20px\">") +
                   name +  QLatin1String("</td>\n");
        content += QLatin1String("    <td style=\"color: ") + settings.headerTextColor +
                   QLatin1String(";\" align=\"right\" bgcolor=\"") + settings.headerBackgroundColor +
                   QLatin1String("\" style=\"padding-right: 20px\">") +
                   birthday + QLatin1String("</td>\n");
        content += QLatin1String("   </tr>\n");

        for (int i = 0; i < blocks.count(); i += 2) {
            // add empty line for spacing
            content += QLatin1String("   <tr>\n");
            content += QLatin1String("    <td>&nbsp;</td>\n");
            content += QLatin1String("    <td>&nbsp;</td>\n");
            content += QLatin1String("   </tr>\n");

            // add real block data
            const ContactBlock leftBlock = blocks.at(i);
            const ContactBlock rightBlock = ((i + 1 < blocks.count()) ?
                                             blocks.at(i + 1) :
                                             ContactBlock());

            content += QLatin1String("   <tr>\n");
            content += QLatin1String("    <td>") + leftBlock.header + QLatin1String("</td>\n");
            content += QLatin1String("    <td>") + rightBlock.header + QLatin1String("</td>\n");
            content += QLatin1String("   </tr>\n");

            const int maxLines = qMax(leftBlock.entries.count(), rightBlock.entries.count());
            for (int j = 0; j < maxLines; ++j) {
                QString leftLine, rightLine;

                if (j < leftBlock.entries.count()) {
                    leftLine = leftBlock.entries.at(j);
                }

                if (j < rightBlock.entries.count()) {
                    rightLine = rightBlock.entries.at(j);
                }

                content += QLatin1String("   <tr>\n");
                content += QLatin1String("    <td class=\"indented\">") + leftLine + QLatin1String("</td>\n");
                content += QLatin1String("    <td class=\"indented\">") + rightLine + QLatin1String("</td>\n");
                content += QLatin1String("   </tr>\n");
            }
        }

        // add empty line for spacing
        content += QLatin1String("   <tr>\n");
        content += QLatin1String("    <td>&nbsp;</td>\n");
        content += QLatin1String("    <td>&nbsp;</td>\n");
        content += QLatin1String("   </tr>\n");
        content += QLatin1String("  </table>\n");
    }
    content += QLatin1String(" </body>\n");
    content += QLatin1String("</html>\n");

    return content;
}

class KABPrinting::AppearancePage : public QWidget, public Ui::AppearancePage_Base
{
public:
    AppearancePage(QWidget *parent)
        : QWidget(parent)
    {
        setupUi(this);
        setObjectName(QLatin1String("AppearancePage"));
    }
};

DetailledPrintStyle::DetailledPrintStyle(PrintingWizard *parent)
    : PrintStyle(parent), mPageAppearance(new AppearancePage(parent))
{
    setPreview(QLatin1String("detailed-style.png"));
    setPreferredSortOptions(ContactFields::FormattedName, Qt::AscendingOrder);

    addPage(mPageAppearance, i18n("Detailed Print Style - Appearance"));

    KConfigGroup config(KSharedConfig::openConfig(), ConfigSectionName);

    mPageAppearance->kcbHeaderBGColor->
    setColor(config.readEntry(ContactHeaderBGColor, QColor(Qt::black)));

    mPageAppearance->kcbHeaderTextColor->
    setColor(config.readEntry(ContactHeaderForeColor, QColor(Qt::white)));

}

DetailledPrintStyle::~DetailledPrintStyle()
{
}

void DetailledPrintStyle::print(const KContacts::Addressee::List &contacts, PrintProgress *progress)
{
    progress->addMessage(i18n("Setting up colors"));
    progress->setProgress(0);

    const QColor headerBackgroundColor = mPageAppearance->kcbHeaderBGColor->color();
    const QColor headerForegroundColor = mPageAppearance->kcbHeaderTextColor->color();

    KConfigGroup config(KSharedConfig::openConfig(), ConfigSectionName);
    config.writeEntry(ContactHeaderForeColor, headerForegroundColor);
    config.writeEntry(ContactHeaderBGColor, headerBackgroundColor);
    config.sync();

    ColorSettings settings;
    settings.headerBackgroundColor = headerBackgroundColor.name();
    settings.headerTextColor = headerForegroundColor.name();

    QPrinter *printer = wizard()->printer();

    progress->addMessage(i18n("Setting up document"));

    const QString html = contactsToHtml(contacts, settings);

    QTextDocument document;
    document.setHtml(html);

    progress->addMessage(i18n("Printing"));

    document.print(printer);

    progress->addMessage(i18nc("Finished printing", "Done"));
}

DetailledPrintStyleFactory::DetailledPrintStyleFactory(PrintingWizard *parent)
    : PrintStyleFactory(parent)
{
}

PrintStyle *DetailledPrintStyleFactory::create() const
{
    return new DetailledPrintStyle(mParent);
}

QString DetailledPrintStyleFactory::description() const
{
    return i18n("Detailed Style");
}

