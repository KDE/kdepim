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
#include <KLineEdit>
#include <KLocale>
#include <kio/global.h>

#include <QPushButton>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QFileInfo>

using namespace PimCommon;

RenameFileDialog::RenameFileDialog(const KUrl& url, bool multiFiles, QWidget * parent)
    :QDialog(parent),
      mUrl(url),
      mApplyAll(0)
{
    setWindowTitle(i18n( "File Already Exists" ));
    QVBoxLayout* pLayout = new QVBoxLayout(this);

    QLabel *label = new QLabel(i18n( "A file named <filename>%1</filename> already exists. Do you want to overwrite it?",url.fileName()),this);
    pLayout->addWidget(label);

    QHBoxLayout* renameLayout = new QHBoxLayout();
    pLayout->addLayout(renameLayout);


    mNameEdit = new KLineEdit(this);
    renameLayout->addWidget(mNameEdit);
    mNameEdit->setText(url.fileName());
    mSuggestNewName = new QPushButton(i18n("Suggest New &Name"), this);
    renameLayout->addWidget(mSuggestNewName);
    connect(mSuggestNewName, SIGNAL(clicked()), this, SLOT(slotSuggestNewNamePressed()));


    QPushButton *overWrite = new QPushButton(i18n("&Overwrite"),this);
    connect(overWrite,SIGNAL(clicked()),this,SLOT(slotOverwritePressed()));

    QPushButton *ignore = new QPushButton(i18n("&Ignore"),this);
    connect(ignore,SIGNAL(clicked()),this,SLOT(slotIgnorePressed()));

    mRename = new QPushButton(i18n("&Rename"),this);
    connect(mRename,SIGNAL(clicked()),this,SLOT(slotRenamePressed()));

    KSeparator* separator = new KSeparator(this);
    pLayout->addWidget(separator);

    QHBoxLayout* layout = new QHBoxLayout();
    pLayout->addLayout(layout);

    if(multiFiles){
        mApplyAll = new QCheckBox(i18n("Appl&y to All"), this);
        connect(mApplyAll, SIGNAL(clicked()), this, SLOT(slotApplyAllPressed()));
        layout->addWidget(mApplyAll);
        slotApplyAllPressed();
    }
    layout->addWidget(mRename);
    layout->addWidget(overWrite);
    layout->addWidget(ignore);
}

RenameFileDialog::~RenameFileDialog()
{
}

void RenameFileDialog::slotOverwritePressed()
{
    if(mApplyAll && mApplyAll->isChecked()) {
        done(RENAMEFILE_OVERWRITEALL);
    } else {
        done(RENAMEFILE_OVERWRITE);
    }
}

void RenameFileDialog::slotIgnorePressed()
{
    if(mApplyAll && mApplyAll->isChecked()) {
        done(RENAMEFILE_IGNOREALL);
    } else {
        done(RENAMEFILE_IGNORE);
    }
}

void RenameFileDialog::slotRenamePressed()
{
    if(mNameEdit->text().isEmpty())
        return;
    done(RENAMEFILE_RENAME);
}

void RenameFileDialog::slotApplyAllPressed()
{
    const bool enabled(!mApplyAll->isChecked());
    mNameEdit->setEnabled(enabled);
    mSuggestNewName->setEnabled(enabled);
    mRename->setEnabled(enabled);
}

void RenameFileDialog::slotSuggestNewNamePressed()
{
    if (mNameEdit->text().isEmpty())
        return;

    KUrl destDirectory(mUrl);

    destDirectory.setPath(destDirectory.directory());
    mNameEdit->setText(suggestName(destDirectory, mNameEdit->text()));
}

KUrl RenameFileDialog::newName() const
{
    KUrl newDest(mUrl);
    QString fileName = mNameEdit->text();

    newDest.setFileName(KIO::encodeFileName(fileName));

    return newDest;
}

QString RenameFileDialog::suggestName(const KUrl& baseURL, const QString& oldName)
{
    QString dotSuffix, suggestedName;
    QString basename = oldName;
    const QChar spacer(' ');

    //ignore dots at the beginning, that way "..aFile.tar.gz" will become "..aFile 1.tar.gz" instead of " 1..aFile.tar.gz"
    int index = basename.indexOf('.');
    int continous = 0;
    while (continous == index) {
        index = basename.indexOf('.', index + 1);
        ++continous;
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
            suggestedName = basename + spacer + '1' + dotSuffix;
        } else {
            // yes there's already a number behind the spacer so increment it by one
            basename.replace(pos + 1, tmp.length(), QString::number(number + 1));
            suggestedName = basename + dotSuffix;
        }
    } else // no spacer yet
        suggestedName = basename + spacer + '1' + dotSuffix ;

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

#include "renamefiledialog.moc"

