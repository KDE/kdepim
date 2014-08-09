/*
 * Copyright (c) 2011-2012-2013 Montel Laurent <montel@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */
#include "renamefiledialog.h"

#include <kseparator.h>
#include <QLineEdit>
#include <QPushButton>
#include <KLocalizedString>
#include <kio/global.h>
#include <kio/netaccess.h>
#include <KMessageBox>

#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QFileInfo>

using namespace PimCommon;


class PimCommon::RenameFileDialog::RenameFileDialogPrivate
{
public:
    RenameFileDialogPrivate(const KUrl& _url, RenameFileDialog *qq)
        : url(_url),
          applyAll(0),
          rename(0),
          suggestNewName(0),
          nameEdit(0),
          q(qq)
    {

    }
    QString suggestName(const KUrl& baseURL, const QString& oldName);

    KUrl url;
    QCheckBox *applyAll;
    QPushButton *rename;
    QPushButton *suggestNewName;
    QLineEdit *nameEdit;
    RenameFileDialog *q;

};

QString PimCommon::RenameFileDialog::RenameFileDialogPrivate::suggestName(const KUrl& baseURL, const QString& oldName)
{
    QString dotSuffix, suggestedName;
    QString basename = oldName;
    const QChar spacer(QLatin1Char(' '));

    //ignore dots at the beginning, that way "..aFile.tar.gz" will become "..aFile 1.tar.gz" instead of " 1..aFile.tar.gz"
    int index = basename.indexOf(QLatin1Char('.'));
    int continuous = 0;
    while (continuous == index) {
        index = basename.indexOf(QLatin1Char('.'), index + 1);
        ++continuous;
    }

    if (index != -1) {
        dotSuffix = basename.mid(index);
        basename.truncate(index);
    }

    const int pos = basename.lastIndexOf(spacer);

    if (pos != -1) {
        const QString tmp = basename.mid(pos + 1);
        bool ok;
        const int number = tmp.toInt(&ok);

        if (!ok) {  // ok there is no number
            suggestedName = basename + spacer + QLatin1Char('1') + dotSuffix;
        } else {
            // yes there's already a number behind the spacer so increment it by one
            basename.replace(pos + 1, tmp.length(), QString::number(number + 1));
            suggestedName = basename + dotSuffix;
        }
    } else // no spacer yet
        suggestedName = basename + spacer + QLatin1Char('1') + dotSuffix ;

    // Check if suggested name already exists
    bool exists = false;
    // TODO: network transparency. However, using NetAccess from a modal dialog
    // could be a problem, no? (given that it uses a modal widget itself....)
    if (baseURL.isLocalFile())
        exists = QFileInfo(baseURL.toLocalFile(KUrl::AddTrailingSlash) + suggestedName).exists();

    if (!exists)
        return suggestedName;
    else // already exists -> recurse
        return suggestName(baseURL, suggestedName);

}


RenameFileDialog::RenameFileDialog(const KUrl& url, bool multiFiles, QWidget * parent)
    : QDialog(parent),
      d(new RenameFileDialogPrivate(url, this))
{
    setWindowTitle(i18n( "File Already Exists" ));
    QVBoxLayout* pLayout = new QVBoxLayout(this);

    QLabel *label = new QLabel(xi18n( "A file named <filename>%1</filename> already exists. Do you want to overwrite it?", url.fileName()),this);
    pLayout->addWidget(label);

    QHBoxLayout* renameLayout = new QHBoxLayout();
    pLayout->addLayout(renameLayout);


    d->nameEdit = new QLineEdit(this);
    renameLayout->addWidget(d->nameEdit);
    d->nameEdit->setClearButtonEnabled(true);
    d->nameEdit->setText(url.fileName());
    d->suggestNewName = new QPushButton(i18n("Suggest New &Name"), this);
    renameLayout->addWidget(d->suggestNewName);
    connect(d->suggestNewName, SIGNAL(clicked()), this, SLOT(slotSuggestNewNamePressed()));


    QPushButton *overWrite = new QPushButton(i18n("&Overwrite"),this);
    connect(overWrite, &QPushButton::clicked, this, &RenameFileDialog::slotOverwritePressed);

    QPushButton *ignore = new QPushButton(i18n("&Ignore"),this);
    connect(ignore, &QPushButton::clicked, this, &RenameFileDialog::slotIgnorePressed);

    d->rename = new QPushButton(i18n("&Rename"),this);
    connect(d->rename,SIGNAL(clicked()),this,SLOT(slotRenamePressed()));

    KSeparator* separator = new KSeparator(this);
    pLayout->addWidget(separator);

    QHBoxLayout* layout = new QHBoxLayout();
    pLayout->addLayout(layout);

    if (multiFiles){
        d->applyAll = new QCheckBox(i18n("Appl&y to All"), this);
        connect(d->applyAll, SIGNAL(clicked()), this, SLOT(slotApplyAllPressed()));
        layout->addWidget(d->applyAll);
        slotApplyAllPressed();
    }
    layout->addWidget(d->rename);
    layout->addWidget(overWrite);
    layout->addWidget(ignore);
}

RenameFileDialog::~RenameFileDialog()
{
    delete d;
}

void RenameFileDialog::slotOverwritePressed()
{
    if (d->applyAll && d->applyAll->isChecked()) {
        done(RENAMEFILE_OVERWRITEALL);
    } else {
        done(RENAMEFILE_OVERWRITE);
    }
}

void RenameFileDialog::slotIgnorePressed()
{
    if (d->applyAll && d->applyAll->isChecked()) {
        done(RENAMEFILE_IGNOREALL);
    } else {
        done(RENAMEFILE_IGNORE);
    }
}

void RenameFileDialog::slotRenamePressed()
{
    if (d->nameEdit->text().isEmpty())
        return;
    const QString name = newName().path();
    if ( KIO::NetAccess::exists( name, KIO::NetAccess::DestinationSide, this ) ) {
        KMessageBox::error(this, i18n("This filename \"%1\" already exists.",name), i18n("File already exists"));
        return;
    }
    done(RENAMEFILE_RENAME);
}

void RenameFileDialog::slotApplyAllPressed()
{
    const bool enabled(!d->applyAll->isChecked());
    d->nameEdit->setEnabled(enabled);
    d->suggestNewName->setEnabled(enabled);
    d->rename->setEnabled(enabled);
}

void RenameFileDialog::slotSuggestNewNamePressed()
{
    if (d->nameEdit->text().isEmpty())
        return;

    KUrl destDirectory(d->url);

    destDirectory.setPath(destDirectory.directory());
    d->nameEdit->setText(d->suggestName(destDirectory, d->nameEdit->text()));
}

KUrl RenameFileDialog::newName() const
{
    KUrl newDest(d->url);
    const QString fileName = d->nameEdit->text();

    newDest.setFileName(KIO::encodeFileName(fileName));

    return newDest;
}



