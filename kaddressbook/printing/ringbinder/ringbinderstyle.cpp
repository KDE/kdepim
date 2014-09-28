/*
  This file is part of KAddressBook.
  Copyright (c) 2002 Jost Schenck <jost@schenck.de>
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

#include "ringbinderstyle.h"
#include "printingwizard.h"
#include "printprogress.h"
#include "ui_rbs_appearance.h"

#include <KConfig>
#include <KLocalizedString>
#include <KConfigGroup>

#include <QPrinter>
#include <QTextDocument>
#include <KSharedConfig>
#include <KLocale>

using namespace KABPrinting;

const char *RingBinderConfigSectionName = "RingBinderPrintStyle";
const char *ShowPhoneNumbers = "ShowPhoneNumbers";
const char *ShowEmailAddresses = "ShowEmailAddresses";
const char *ShowStreetAddresses = "ShowStreetAddresses";
const char *ShowOrganization = "ShowOrganization";
const char *ShowBirthday = "ShowBirthday";
const char *ShowNote = "ShowNote";

enum PrintField {
    PhoneNumbers = 1,
    Emails = 2,
    Addresses = 4,
    Organization = 8,
    Birthday = 16,
    Note = 32
};

static QString contactsToHtml(const KABC::Addressee::List &contacts, int fields)
{
    QString content;

    content += QLatin1String("<html>\n");
    content += QLatin1String(" <body>\n");
    content += QLatin1String("  <table style=\"border-width: 1px; border-style: solid; "
                             "border-color: gray;\" width=\"100%\" cellspacing=\"0\">\n");
    foreach (const KABC::Addressee &contact, contacts) {
        QString nameString = contact.familyName() + QLatin1String(", ") + contact.givenName();

        if (fields & Organization) {
            if (!contact.organization().isEmpty()) {
                nameString += QLatin1String(" (") + contact.organization() + QLatin1Char(')');
            }
        }

        if (fields & Birthday) {
            if (contact.birthday().isValid()) {
                nameString += QLatin1String(" *") + KLocale::global()->formatDate(contact.birthday().date(),
                              KLocale::ShortDate);
            }
        }

        QStringList leftBlock, rightBlock;
        if (fields & PhoneNumbers) {
            const KABC::PhoneNumber::List numbers = contact.phoneNumbers();
            foreach (const KABC::PhoneNumber &number, numbers) {
                rightBlock.append(number.typeLabel() + QLatin1String(": ") + number.number());
            }
        }
        if (fields & Emails) {
            const QStringList emails = contact.emails();
            foreach (const QString &email, emails) {
                rightBlock.append(email);
            }
        }
        if (fields & Note) {
            if (!contact.note().isEmpty()) {
                const QString note = QLatin1String("Note: ") + contact.note().replace(QLatin1Char('\n'), QLatin1String("<br/>"));

                rightBlock.append(note);
            }
        }
        if (fields & Addresses) {
            const KABC::Address::List addresses = contact.addresses();
            foreach (const KABC::Address &address, addresses) {
                const QString data =
                    address.formattedAddress().replace(QLatin1String("\n\n"), QLatin1String("\n")).replace(QLatin1Char('\n'), QLatin1String("<br/>"));
                const QString subBlock = QLatin1String("<p style=\"margin-top: 0px; margin-left: 20px\">") + data + QLatin1String("</p>");

                leftBlock.append(subBlock);
            }
        }

        content += QLatin1String("   <tr>\n");
        content += QLatin1String("    <td style=\"padding-left: 3px; padding-top: 3px; padding-right: 3px; "
                                 "padding-bottom: 3px;\">") +
                   nameString + leftBlock.join(QString()) + QLatin1String("</td>\n");
        content += QLatin1String("    <td style=\"padding-left: 3px; padding-top: 3px; padding-right: 3px; "
                                 "padding-bottom: 3px;\">") +
                   rightBlock.join(QLatin1String("<br/>")) + QLatin1String("</td>\n");
        content += QLatin1String("   </tr>\n");
    }
    content += QLatin1String("  </table>\n");
    content += QLatin1String(" </body>\n");
    content += QLatin1String("</html>\n");

    return content;
}

namespace KABPrinting
{

class RingBinderStyleAppearanceForm : public QWidget, public Ui::RingBinderStyleAppearanceForm_Base
{
public:
    explicit RingBinderStyleAppearanceForm(QWidget *parent)
        : QWidget(parent)
    {
        setObjectName(QLatin1String("AppearancePage"));
        setupUi(this);
    }
};

}

RingBinderPrintStyle::RingBinderPrintStyle(PrintingWizard *parent)
    : PrintStyle(parent),
      mPageAppearance(new RingBinderStyleAppearanceForm(parent))
{
    setPreview(QLatin1String("ringbinder-style.png"));
    setPreferredSortOptions(ContactFields::FamilyName, Qt::AscendingOrder);

    addPage(mPageAppearance, i18n("Ring Binder Printing Style - Appearance"));

    // applying previous settings
    KConfigGroup config(KSharedConfig::openConfig(), RingBinderConfigSectionName);
    mPageAppearance->cbPhoneNumbers->setChecked(config.readEntry(ShowPhoneNumbers, true));
    mPageAppearance->cbEmails->setChecked(config.readEntry(ShowEmailAddresses, true));
    mPageAppearance->cbStreetAddresses->setChecked(config.readEntry(ShowStreetAddresses, true));
    mPageAppearance->cbOrganization->setChecked(config.readEntry(ShowOrganization, true));
    mPageAppearance->cbBirthday->setChecked(config.readEntry(ShowBirthday, false));
    mPageAppearance->cbNote->setChecked(config.readEntry(ShowNote, false));
}

RingBinderPrintStyle::~RingBinderPrintStyle()
{
}

void RingBinderPrintStyle::print(const KABC::Addressee::List &contacts, PrintProgress *progress)
{
    progress->addMessage(i18n("Setting up fields"));
    progress->setProgress(0);

    // first write current config settings
    KConfigGroup config(KSharedConfig::openConfig(), RingBinderConfigSectionName);
    config.writeEntry(ShowPhoneNumbers, mPageAppearance->cbPhoneNumbers->isChecked());
    config.writeEntry(ShowEmailAddresses, mPageAppearance->cbEmails->isChecked());
    config.writeEntry(ShowStreetAddresses, mPageAppearance->cbStreetAddresses->isChecked());
    config.writeEntry(ShowOrganization, mPageAppearance->cbOrganization->isChecked());
    config.writeEntry(ShowBirthday, mPageAppearance->cbBirthday->isChecked());
    config.writeEntry(ShowNote, mPageAppearance->cbNote->isChecked());
    config.sync();

    QPrinter *printer = wizard()->printer();
    printer->setPageMargins(50, 20, 0, 50, QPrinter::DevicePixel);

    progress->addMessage(i18n("Setting up document"));

    int fields = 0;

    if (mPageAppearance->cbPhoneNumbers->isChecked()) {
        fields |= PhoneNumbers;
    }

    if (mPageAppearance->cbEmails->isChecked()) {
        fields |= Emails;
    }

    if (mPageAppearance->cbStreetAddresses->isChecked()) {
        fields |= Addresses;
    }

    if (mPageAppearance->cbOrganization->isChecked()) {
        fields |= Organization;
    }

    if (mPageAppearance->cbBirthday->isChecked()) {
        fields |= Birthday;
    }

    if (mPageAppearance->cbNote->isChecked()) {
        fields |= Note;
    }

    const QString html = contactsToHtml(contacts, fields);

    QTextDocument document;
    document.setHtml(html);

    progress->addMessage(i18n("Printing"));

    document.print(printer);

    progress->addMessage(i18nc("Finished printing", "Done"));
}

RingBinderPrintStyleFactory::RingBinderPrintStyleFactory(PrintingWizard *parent)
    : PrintStyleFactory(parent)
{
}

PrintStyle *RingBinderPrintStyleFactory::create() const
{
    return new RingBinderPrintStyle(mParent);
}

QString RingBinderPrintStyleFactory::description() const
{
    return i18n("Printout for Ring Binders");
}
