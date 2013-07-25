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

#include "scamdetectiondetailsdialog.h"
#include "settings/globalsettings.h"
#include "utils/autoqpointer.h"

#include <KLocale>

#include <KTextEdit>
#include <KFileDialog>
#include <KStandardGuiItem>

#include <QTextStream>

using namespace MessageViewer;

ScamDetectionDetailsDialog::ScamDetectionDetailsDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n("Details") );
    setAttribute( Qt::WA_DeleteOnClose );
    setButtons( User1|Close );
    setButtonGuiItem( User1, KStandardGuiItem::saveAs() );
    setModal( false );
    mDetails = new KTextEdit;
    mDetails->setReadOnly(true);
    mDetails->setAcceptRichText(true);
    setMainWidget(mDetails);
    connect(this, SIGNAL(user1Clicked()), SLOT(slotSaveAs()));
    readConfig();
}

ScamDetectionDetailsDialog::~ScamDetectionDetailsDialog()
{
    writeConfig();
}

void ScamDetectionDetailsDialog::slotSaveAs()
{
    KUrl url;
    MessageViewer::AutoQPointer<KFileDialog> fdlg( new KFileDialog( url, QString(), this) );

    fdlg->setMode( KFile::File );
    fdlg->setSelection( QLatin1String("scam-detection.html") );
    fdlg->setOperationMode( KFileDialog::Saving );
    fdlg->setConfirmOverwrite(true);
    if ( fdlg->exec() == QDialog::Accepted ) {
        const QString fileName = fdlg->selectedFile();
        if ( !fileName.isEmpty() ) {
            QFile file(fileName);
            if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
                kDebug()<<"We can't save in file :"<<fileName;
                return;
            }
            QTextStream ts( &file );
            ts.setCodec("UTF-8");
            ts << mDetails->toHtml();
            file.close();
        }
    }
}

void ScamDetectionDetailsDialog::setDetails(const QString &details)
{
    mDetails->setHtml(details);
}

void ScamDetectionDetailsDialog::readConfig()
{
    KConfigGroup group( MessageViewer::GlobalSettings::self()->config(), "ScamDetectionDetailsDialog" );
    const QSize size = group.readEntry( "Size", QSize() );
    if ( size.isValid() ) {
        resize( size );
    } else {
        resize( 600, 400 );
    }
}

void ScamDetectionDetailsDialog::writeConfig()
{
    KConfigGroup group( MessageViewer::GlobalSettings::self()->config(), "ScamDetectionDetailsDialog" );
    group.writeEntry( "Size", size() );
    group.sync();
}

#include "scamdetectiondetailsdialog.moc"
