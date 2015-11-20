/*
  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#include "attachmenteditdialog.h"
#include "attachmenticonview.h"
#include "ui_attachmenteditdialog.h"
#include <KLocalizedString>
#include <KIO/StoredTransferJob>
#include <KJobWidgets>
#include <KLocale>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMimeDatabase>
#include <QMimeType>

using namespace IncidenceEditorNG;

AttachmentEditDialog::AttachmentEditDialog(AttachmentIconItem *item,
        QWidget *parent,
        bool modal)
    : QDialog(parent),
#ifdef KDEPIM_ENTERPRISE_BUILD
      mAttachment(new KCalCore::Attachment('\0')),     //use the non-uri constructor
      // as we want inline by default
#else
      mAttachment(new KCalCore::Attachment(QString())),
#endif
      mItem(item),
      mUi(new Ui::AttachmentEditDialog)
{
    QMimeDatabase db;
    mMimeType = db.mimeTypeForName(item->mimeType());
    QWidget *page = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AttachmentEditDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AttachmentEditDialog::reject);
    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox);

    mUi->setupUi(page);
    mUi->mLabelEdit->setText(item->label().isEmpty() ? item->uri() : item->label());
    mUi->mIcon->setPixmap(item->icon());
    mUi->mInlineCheck->setChecked(item->isBinary());

    QString typecomment = item->mimeType().isEmpty() ?
                          i18nc("@label unknown mimetype", "Unknown") :
                          mMimeType.comment();
    mUi->mTypeLabel->setText(typecomment);

    setModal(modal);
    mOkButton->setEnabled(false);

    mUi->mInlineCheck->setEnabled(false);
    if (item->attachment()->isUri() || item->attachment()->data().isEmpty()) {
        mUi->mStackedWidget->setCurrentIndex(0);
        mUi->mURLRequester->setUrl(item->uri());
        urlChanged(item->uri());
    } else {
        mUi->mInlineCheck->setEnabled(true);
        mUi->mStackedWidget->setCurrentIndex(1);
        mUi->mSizeLabel->setText(QStringLiteral("%1 (%2)").
                                 arg(KIO::convertSize(item->attachment()->size())).
                                 arg(KLocale::global()->formatNumber(
                                         item->attachment()->size(), 0)));
    }

    connect(mUi->mInlineCheck, &QCheckBox::stateChanged, this, &AttachmentEditDialog::inlineChanged);
    connect(mUi->mURLRequester, static_cast<void (KUrlRequester::*)(const QUrl &)>(&KUrlRequester::urlSelected),
            this, static_cast<void (AttachmentEditDialog::*)(const QUrl &)>(&AttachmentEditDialog::urlChanged));
    connect(mUi->mURLRequester, &KUrlRequester::textChanged,
            this, static_cast<void (AttachmentEditDialog::*)(const QString &)>(&AttachmentEditDialog::urlChanged));
}

AttachmentEditDialog::~AttachmentEditDialog()
{
    delete mUi;
}

void AttachmentEditDialog::accept()
{
    slotApply();
    QDialog::accept();
}

void AttachmentEditDialog::slotApply()
{
    QUrl url = mUi->mURLRequester->url();

    if (mUi->mLabelEdit->text().isEmpty()) {
        if (url.isLocalFile()) {
            mItem->setLabel(url.fileName());
        } else {
            mItem->setLabel(url.url());
        }
    } else {
        mItem->setLabel(mUi->mLabelEdit->text());
    }
    if (mItem->label().isEmpty()) {
        mItem->setLabel(i18nc("@label", "New attachment"));
    }
    mItem->setMimeType(mMimeType.name());

    QString correctedUrl = url.url();
    if (!url.isEmpty() && url.isRelative()) {
        // If the user used KURLRequester's KURLCompletion
        // (used the line edit instead of the file dialog)
        // the returned url is not absolute and is always relative
        // to the home directory (not pwd), so we must prepend home

        correctedUrl = QDir::home().filePath(url.toLocalFile());
        url = QUrl::fromLocalFile(correctedUrl);
        if (url.isValid()) {
            urlChanged(url);
            mItem->setLabel(url.fileName());
            mItem->setUri(correctedUrl);
            mItem->setMimeType(mMimeType.name());
        }
    }

    if (mUi->mStackedWidget->currentIndex() == 0) {
        if (mUi->mInlineCheck->isChecked()) {
            auto job = KIO::storedGet(url);
            KJobWidgets::setWindow(job, 0);
            if (job->exec()) {
                QByteArray data  = job->data();
                mItem->setData(data);
            }
        } else {
            mItem->setUri(correctedUrl);
        }
    }
}

void AttachmentEditDialog::inlineChanged(int state)
{
    mOkButton->setEnabled(!mUi->mURLRequester->url().toDisplayString().trimmed().isEmpty() ||
                          mUi->mStackedWidget->currentIndex() == 1);
    if (state == Qt::Unchecked && mUi->mStackedWidget->currentIndex() == 1) {
        mUi->mStackedWidget->setCurrentIndex(0);
        if (!mItem->savedUri().isEmpty()) {
            mUi->mURLRequester->setUrl(mItem->savedUri());
        } else {
            mUi->mURLRequester->setUrl(mItem->uri());
        }
    }
}

void AttachmentEditDialog::urlChanged(const QString &url)
{
    const bool urlIsNotEmpty = !url.trimmed().isEmpty();
    mOkButton->setEnabled(urlIsNotEmpty);
    mUi->mInlineCheck->setEnabled(urlIsNotEmpty ||
                                  mUi->mStackedWidget->currentIndex() == 1);
}

void AttachmentEditDialog::urlChanged(const QUrl &url)
{
    QMimeDatabase db;
    mMimeType = db.mimeTypeForUrl(url);
    mUi->mTypeLabel->setText(mMimeType.comment());
    mUi->mIcon->setPixmap(AttachmentIconItem::icon(mMimeType, url.path()));
}

