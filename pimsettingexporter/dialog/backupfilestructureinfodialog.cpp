/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include <KLocale>
#include <KStandardDirs>
#include <KMessageBox>

#include <QHBoxLayout>
#include <QFile>
#include <QLabel>
#include <QDebug>

BackupFileStructureInfoDialog::BackupFileStructureInfoDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Archive File Structure" ) );
    setButtons( Close );
    setDefaultButton( Close );
    setModal( true );

    QWidget *w = new QWidget;

    QVBoxLayout *lay = new QVBoxLayout;
    w->setLayout(lay);

    QLabel *lab = new QLabel(i18n("Backup Archive Structure:"));
    lay->addWidget(lab);

    mEditor = new PimCommon::PlainTextEditorWidget;
    mEditor->setReadOnly(true);
    lay->addWidget(mEditor);
    setMainWidget(w);
    loadStructure();
    readConfig();
}

BackupFileStructureInfoDialog::~BackupFileStructureInfoDialog()
{
    writeConfig();
}

void BackupFileStructureInfoDialog::loadStructure()
{
    const QString fileName( KStandardDirs::locate( "data", QLatin1String("pimsettingexporter/backup-structure.txt") ) );
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
    KConfigGroup group( KGlobal::config(), "BackupFileStructureInfoDialog" );
    group.writeEntry( "Size", size() );
}

void BackupFileStructureInfoDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "BackupFileStructureInfoDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(600,400) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}


#include "backupfilestructureinfodialog.moc"
