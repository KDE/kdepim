/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "grantleeprintstyle.h"
#include "contactfields.h"
#include "printingwizard.h"
#include "printprogress.h"
#include "printstyle.h"
#include "kaddressbookgrantlee/printing/grantleeprint.h"

#include <KContacts/Addressee>

#include <KLocalizedString>

#include <QPrinter>
#include <QTextDocument>
#include <QFile>
#include <QDir>

using namespace KABPrinting;

GrantleePrintStyle::GrantleePrintStyle(const QString &themePath, PrintingWizard *parent)
    : PrintStyle(parent)
{
    mGrantleePrint = new KAddressBookGrantlee::GrantleePrint(themePath, this);
    QFile previewFile(QString(themePath + QDir::separator() + QStringLiteral("preview.png")));
    if (previewFile.exists()) {
        setPreview(previewFile.fileName());
    }
    setPreferredSortOptions(ContactFields::FormattedName, Qt::AscendingOrder);
}

GrantleePrintStyle::~GrantleePrintStyle()
{
}

void GrantleePrintStyle::print(const KContacts::Addressee::List &contacts, PrintProgress *progress)
{
    QPrinter *printer = wizard()->printer();
    printer->setPageMargins(20, 20, 20, 20, QPrinter::DevicePixel);

    progress->addMessage(i18n("Setting up document"));

    const QString html = mGrantleePrint->contactsToHtml(contacts);

    QTextDocument document;
    document.setHtml(html);

    progress->addMessage(i18n("Printing"));

    document.print(printer);

    progress->addMessage(i18nc("Finished printing", "Done"));
}

GrantleeStyleFactory::GrantleeStyleFactory(const QString &name, const QString &themePath, PrintingWizard *parent)
    : PrintStyleFactory(parent),
      mThemePath(themePath),
      mName(name)
{
}

PrintStyle *GrantleeStyleFactory::create() const
{
    return new GrantleePrintStyle(mThemePath, mParent);
}

QString GrantleeStyleFactory::description() const
{
    return mName;
}

