/*
  This file is part of KAddressBook.
  Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>
                          Tobias Koenig <tokoe@kde.org>

  Copyright (c) 2009-2015 Laurent Montel <montel@kde.org>
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

#include "printingwizard.h"
#include "settings.h"

#include "contactselectionwidget.h"
#include "contactsorter.h"
#include "printprogress.h"
#include "printstyle.h"
#include "stylepage.h"

// including the styles
#include "detailled/detailledstyle.h"
#include "mike/mikesstyle.h"
#include "ringbinder/ringbinderstyle.h"
#include "compact/compactstyle.h"
#include "printing/grantlee/grantleeprintstyle.h"

#include <QApplication>
#include "kaddressbook_debug.h"
#include <KLocalizedString>

#include <QPushButton>
#include <QPrinter>
#include <QDirIterator>
#include <KSharedConfig>
#include <QStandardPaths>

using namespace KABPrinting;

PrintingWizard::PrintingWizard(QPrinter *printer, QItemSelectionModel *selectionModel,
                               QWidget *parent)
    : KAssistantDialog(parent), mPrinter(printer), mStyle(Q_NULLPTR)
{
    setWindowTitle(i18n("Print Contacts"));

    mSelectionPage = new ContactSelectionWidget(selectionModel, this);
    mSelectionPage->setMessageText(i18n("Which contacts do you want to print?"));

    KPageWidgetItem *mSelectionPageItem =
        new KPageWidgetItem(mSelectionPage, i18n("Choose Contacts to Print"));
    addPage(mSelectionPageItem);
    setAppropriate(mSelectionPageItem, true);

    mStylePage = new StylePage(this);
    connect(mStylePage, &StylePage::styleChanged, this, &PrintingWizard::slotStyleSelected);
    addPage(mStylePage, i18n("Choose Printing Style"));

    registerStyles();

    if (mStyleFactories.count() > Settings::self()->printingStyle()) {
        mStylePage->setPrintingStyle(Settings::self()->printingStyle());   // should Q_EMIT styleChanged
        slotStyleSelected(Settings::self()->printingStyle());
    }

    mStylePage->setSortOrder(Settings::self()->sortOrder() == 0 ?
                             Qt::AscendingOrder :
                             Qt::DescendingOrder);
    readConfig();
}

PrintingWizard::~PrintingWizard()
{
    writeConfig();
}

void PrintingWizard::readConfig()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "PrintingWizard");
    const QSize size = grp.readEntry("Size", QSize(300, 200));
    if (size.isValid()) {
        resize(size);
    }
}

void PrintingWizard::writeConfig()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "PrintingWizard");
    grp.writeEntry("Size", size());
    grp.sync();
}

void PrintingWizard::setDefaultAddressBook(const Akonadi::Collection &addressBook)
{
    mSelectionPage->setDefaultAddressBook(addressBook);
}

void PrintingWizard::accept()
{
    print();
    close();
    setResult(QDialog::Accepted);
}

void PrintingWizard::loadGrantleeStyle()
{
    const QString relativePath = QStringLiteral("kaddressbook/printing/themes/");
    QStringList themesDirectories = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, relativePath, QStandardPaths::LocateDirectory);
    if (themesDirectories.count() < 2) {
        //Make sure to add local directory
        const QString localDirectory = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + relativePath;
        if (!themesDirectories.contains(localDirectory)) {
            themesDirectories.append(localDirectory);
        }
    }

    Q_FOREACH (const QString &directory, themesDirectories) {
        QDirIterator dirIt(directory, QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot);
        QStringList alreadyLoadedThemeName;
        while (dirIt.hasNext()) {
            dirIt.next();
            const QString themeInfoFile = dirIt.filePath() + QDir::separator() + QStringLiteral("theme.desktop");
            KConfig config(themeInfoFile);
            KConfigGroup group(&config, QStringLiteral("Desktop Entry"));
            QString name = group.readEntry("Name", QString());
            if (name.isEmpty()) {
                continue;
            }
            if (alreadyLoadedThemeName.contains(name)) {
                int i = 2;
                const QString originalName(name);
                while (alreadyLoadedThemeName.contains(name)) {
                    name = originalName + QStringLiteral(" (%1)").arg(i);
                    ++i;
                }
            }
            const QString printThemePath(dirIt.filePath() + QDir::separator());
            if (!printThemePath.isEmpty()) {
                alreadyLoadedThemeName << name;
                mStyleFactories.append(new GrantleeStyleFactory(name, printThemePath, this));
            }
        }
    }
}

void PrintingWizard::registerStyles()
{
    mStyleFactories.append(new DetailledPrintStyleFactory(this));
    mStyleFactories.append(new MikesStyleFactory(this));
    mStyleFactories.append(new RingBinderPrintStyleFactory(this));
    mStyleFactories.append(new CompactStyleFactory(this));

    loadGrantleeStyle();

    mStylePage->clearStyleNames();
    for (int i = 0; i < mStyleFactories.count(); ++i) {
        mStylePage->addStyleName(mStyleFactories.at(i)->description());
    }
}

void PrintingWizard::slotStyleSelected(int index)
{
    if (index < 0 || index >= mStyleFactories.count()) {
        return;
    }

    if (mStyle) {
        mStyle->hidePages();
    }

    mStyle = mStyleList.value(index);
    if (!mStyle) {
        PrintStyleFactory *factory = mStyleFactories.at(index);
        qCDebug(KADDRESSBOOK_LOG) << "creating print style" << factory->description();

        mStyle = factory->create();
        mStyleList.insert(index, mStyle);
    }

    mStyle->showPages();

    mStylePage->setPreview(mStyle->preview());

    mStylePage->setSortField(mStyle->preferredSortField());
    mStylePage->setSortOrder(mStyle->preferredSortOrder());
}

QPrinter *PrintingWizard::printer()
{
    return mPrinter;
}

int PrintingWizard::printingStyle() const
{
    return mStylePage->printingStyle();
}

int PrintingWizard::sortOrder() const
{
    return mStylePage->sortOrder();
}

void PrintingWizard::print()
{
    // create and show print progress widget:
    mProgress = new PrintProgress(this);
    KPageWidgetItem *progressItem = new KPageWidgetItem(mProgress, i18n("Print Progress"));
    addPage(progressItem);
    setCurrentPage(progressItem);
    qApp->processEvents();

    KContacts::Addressee::List contacts = mSelectionPage->selectedContacts().addressList();

    const ContactSorter sorter(mStylePage->sortField(), mStylePage->sortOrder());
    sorter.sort(contacts);

    qCDebug(KADDRESSBOOK_LOG) << "printing" << contacts.count() << "contacts.";
    // ... print:
    backButton()->setEnabled(false);

    button(QDialogButtonBox::Cancel)->setEnabled(false);
    mStyle->print(contacts, mProgress);
}

