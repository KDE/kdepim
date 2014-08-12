/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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


#include "backupfilestructureinfodialog.h"

#include "pimcommon/texteditor/plaintexteditor/plaintexteditorwidget.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KStandardDirs>
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
    setWindowTitle( i18n( "Archive File Structure" ) );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    buttonBox->button(QDialogButtonBox::Close)->setDefault(true);
    setModal( true );

    QWidget *w = new QWidget;

    QVBoxLayout *lay = new QVBoxLayout;
    w->setLayout(lay);

    QLabel *lab = new QLabel(i18n("Backup Archive Structure:"));
    lay->addWidget(lab);

    mEditor = new PimCommon::PlainTextEditorWidget;
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
    const QString fileName( QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("pimsettingexporter/backup-structure.txt") ) );
    if (!fileName.isEmpty()) {
        QFile f(fileName);
        if (!f.open(QIODevice::ReadOnly)) {
            KMessageBox::error(this, i18n("backup-structure.txt file was not found."));
            return;
        }
        mEditor->setPlainText(QString::fromLatin1(f.readAll()));
    }
}

void BackupFileStructureInfoDialog::writeConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "BackupFileStructureInfoDialog" );
    group.writeEntry( "Size", size() );
}

void BackupFileStructureInfoDialog::readConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "BackupFileStructureInfoDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(600,400) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}


