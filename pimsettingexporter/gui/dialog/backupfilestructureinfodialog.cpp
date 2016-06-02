/*
   Copyright (C) 2013-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "backupfilestructureinfodialog.h"

#include "kpimtextedit/plaintexteditorwidget.h"
#include "kpimtextedit/plaintexteditor.h"

#include <KLocalizedString>
#include <KSharedConfig>

#include <KMessageBox>

#include <QFile>
#include <QLabel>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>

BackupFileStructureInfoDialog::BackupFileStructureInfoDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Archive File Structure"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &BackupFileStructureInfoDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &BackupFileStructureInfoDialog::reject);
    buttonBox->button(QDialogButtonBox::Close)->setDefault(true);
    setModal(true);

    QWidget *w = new QWidget;

    QVBoxLayout *lay = new QVBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    QLabel *lab = new QLabel(i18n("Backup Archive Structure:"));
    lay->addWidget(lab);

    mEditor = new KPIMTextEdit::PlainTextEditorWidget;
    mEditor->editor()->setWebShortcutSupport(false);
    mEditor->setReadOnly(true);
    lay->addWidget(mEditor);
    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);
    loadStructure();
    readConfig();
}

BackupFileStructureInfoDialog::~BackupFileStructureInfoDialog()
{
    writeConfig();
}

void BackupFileStructureInfoDialog::loadStructure()
{
    QFile f(QStringLiteral(":/structure/backup-structure.txt"));
    if (!f.open(QIODevice::ReadOnly)) {
        KMessageBox::error(this, i18n("backup-structure.txt file was not found."));
        return;
    }
    mEditor->setPlainText(QString::fromLatin1(f.readAll()));
}

void BackupFileStructureInfoDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "BackupFileStructureInfoDialog");
    group.writeEntry("Size", size());
}

void BackupFileStructureInfoDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "BackupFileStructureInfoDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(600, 400));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

