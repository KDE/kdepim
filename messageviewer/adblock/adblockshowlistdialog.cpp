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

#include "adblockshowlistdialog.h"
#include "adblocksyntaxhighlighter.h"
#include "pimcommon/plaintexteditor/plaintexteditorwidget.h"
#include "pimcommon/plaintexteditor/plaintexteditor.h"

#include <KPIMUtils/ProgressIndicatorLabel>

#include <KLocale>
#include <KIO/Job>
#include <KTemporaryFile>

#include <QHBoxLayout>
#include <KDebug>

using namespace MessageViewer;
AdBlockShowListDialog::AdBlockShowListDialog(QWidget *parent)
    : KDialog(parent),
      mTemporaryFile(0)
{
    setCaption( i18n("Show adblock list") );
    setButtons( Close );
    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    mTextEdit = new PimCommon::PlainTextEditorWidget;
    (void)new MessageViewer::AdBlockSyntaxHighlighter(mTextEdit->editor()->document());
    mTextEdit->setReadOnly(true);
    lay->addWidget(mTextEdit);

    mProgress = new KPIMUtils::ProgressIndicatorLabel(i18n("Download..."));
    lay->addWidget(mProgress);
    w->setLayout(lay);
    setMainWidget(w);
    readConfig();
}

AdBlockShowListDialog::~AdBlockShowListDialog()
{
    delete mTemporaryFile;
    writeConfig();
}

void AdBlockShowListDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "AdBlockShowListDialog" );
    group.writeEntry( "Size", size() );
}

void AdBlockShowListDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "AdBlockShowListDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(800,600) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

void AdBlockShowListDialog::setAdBlockListPath(const QString &localPath, const QString &url)
{
    if (localPath.isEmpty()) {
        QFile file(localPath);
        if (file.exists()) {
            mTextEdit->editor()->setPlainText(QString::fromUtf8(file.readAll()));
        } else {
            downLoadList(url);
        }
    } else {
        downLoadList(url);
    }
}

void AdBlockShowListDialog::downLoadList(const QString &url)
{
    delete mTemporaryFile;
    mTemporaryFile = new KTemporaryFile;
    if (!mTemporaryFile->open()) {
        kDebug()<<"can not open temporary file";
        delete mTemporaryFile;
        mTemporaryFile = 0;
        return;
    }
    KUrl subUrl = KUrl(url);

    KUrl destUrl = KUrl(mTemporaryFile->fileName());

    mProgress->start();
    KIO::FileCopyJob* job = KIO::file_copy(subUrl , destUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
    job->metaData().insert(QLatin1String("ssl_no_client_cert"), QLatin1String("TRUE"));
    job->metaData().insert(QLatin1String("ssl_no_ui"), QLatin1String("TRUE"));
    job->metaData().insert(QLatin1String("UseCache"), QLatin1String("false"));
    job->metaData().insert(QLatin1String("cookies"), QLatin1String("none"));
    job->metaData().insert(QLatin1String("no-auth"), QLatin1String("true"));

    connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotFinished(KJob*)));
}

void AdBlockShowListDialog::slotFinished(KJob *job)
{
    mProgress->stop();
    if (job->error()) {
        mTemporaryFile->close();
        delete mTemporaryFile;
        mTemporaryFile = 0;
        mTextEdit->editor()->setPlainText(i18n("An error occurs during download list: \"%1\"", job->errorString()));
        return;
    }

    QFile f(mTemporaryFile->fileName());
    if (!f.open(QIODevice::ReadOnly|QIODevice::Text)) {
        mTemporaryFile->close();
        delete mTemporaryFile;
        mTemporaryFile = 0;
        return;
    }
    mTextEdit->editor()->setPlainText(QString::fromUtf8(f.readAll()));
    mTemporaryFile->close();
}


#include "adblockshowlistdialog.moc"
