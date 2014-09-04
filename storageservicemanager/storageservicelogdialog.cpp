/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "storageservicelogdialog.h"
#include "pimcommon/texteditor/richtexteditor/richtexteditorwidget.h"
#include "pimcommon/texteditor/richtexteditor/richtexteditor.h"
#include "pimcommon/util/pimutil.h"

#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

StorageServiceLogDialog::StorageServiceLogDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Log"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    mUser1Button = new QPushButton;
    buttonBox->addButton(mUser1Button, QDialogButtonBox::ActionRole);
    mUser2Button = new QPushButton;
    buttonBox->addButton(mUser2Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &StorageServiceLogDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &StorageServiceLogDialog::reject);
    mUser1Button->setText(i18n("Clear Log"));
    mUser2Button->setText(i18n("Save As..."));
    mLog = new PimCommon::RichTextEditorWidget;
    mLog->setReadOnly(true);
    readConfig();
    mainLayout->addWidget(mLog);
    mainLayout->addWidget(buttonBox);
    connect(mUser1Button, &QPushButton::clicked, this, &StorageServiceLogDialog::slotClearLog);
    connect(mUser2Button, &QPushButton::clicked, this, &StorageServiceLogDialog::slotSaveAs);
    connect(mLog->editor(), SIGNAL(textChanged()), this, SLOT(slotTextChanged()));
}

StorageServiceLogDialog::~StorageServiceLogDialog()
{
    writeConfig();
}

void StorageServiceLogDialog::slotTextChanged()
{
    const bool status = !mLog->toPlainText().isEmpty();
    mUser2Button->setEnabled(status);
    mUser1Button->setEnabled(status);
}

void StorageServiceLogDialog::slotClearLog()
{
    mLog->editor()->clear();
    Q_EMIT clearLog();
    close();
}

void StorageServiceLogDialog::slotSaveAs()
{
    const QString filter = QLatin1String("*|") + i18n("all files (*)");
    PimCommon::Util::saveTextAs(mLog->toPlainText(), filter, this, KUrl(), i18n("Save Log"));
}

void StorageServiceLogDialog::setLog(const QString &log)
{
    mLog->setHtml(log);
}

void StorageServiceLogDialog::writeConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    KConfigGroup group = config->group(QLatin1String("StorageServiceLogDialog"));
    group.writeEntry("Size", size());
}

void StorageServiceLogDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "StorageServiceLogDialog");
    const QSize size = group.readEntry("Size", QSize(600, 400));
    if (size.isValid()) {
        resize(size);
    }
}
