/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include "importmailpage.h"
#include "ui_importmailpage.h"

ImportMailPage::ImportMailPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImportMailPage)
{
    ui->setupUi(this);
    connect(ui->importMails, &QPushButton::clicked, this, &ImportMailPage::importMailsClicked);
    connect(ui->mCollectionRequestor, &MailCommon::FolderRequester::folderChanged, this, &ImportMailPage::collectionChanged);
}

ImportMailPage::~ImportMailPage()
{
    delete ui;
}

MailImporter::ImportMailsWidget *ImportMailPage::mailWidget()
{
    return ui->mMailImporterWidget;
}

void ImportMailPage::collectionChanged(const Akonadi::Collection &collection)
{
    ui->importMails->setEnabled(collection.isValid());
}

Akonadi::Collection ImportMailPage::selectedCollection() const
{
    return ui->mCollectionRequestor->collection();
}

void ImportMailPage::setImportButtonEnabled(bool enabled)
{
    ui->importMails->setEnabled(enabled);
}

