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


StorageServiceLogDialog::StorageServiceLogDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption(i18n("Log"));

    setButtons( User2 | User1 | Close );
    setButtonText( User1, i18n("Clear Log"));
    setButtonText( User2, i18n("Save As..."));
    mLog = new PimCommon::RichTextEditorWidget;
    mLog->setReadOnly(true);
    readConfig();
    setMainWidget(mLog);
    connect(this, &StorageServiceLogDialog::user1Clicked, this, &StorageServiceLogDialog::slotClearLog);
    connect(this, &StorageServiceLogDialog::user2Clicked, this, &StorageServiceLogDialog::slotSaveAs);
    connect(mLog->editor(), SIGNAL(textChanged()), this, SLOT(slotTextChanged()));
}

StorageServiceLogDialog::~StorageServiceLogDialog()
{
    writeConfig();
}

void StorageServiceLogDialog::slotTextChanged()
{
    const bool status = !mLog->toPlainText().isEmpty();
    enableButton(User2, status);
    enableButton(User1, status);
}

void StorageServiceLogDialog::slotClearLog()
{
    mLog->editor()->clear();
    Q_EMIT clearLog();
    close();
}

void StorageServiceLogDialog::slotSaveAs()
{
    const QString filter = QLatin1String("*|") + i18n( "all files (*)" );
    PimCommon::Util::saveTextAs(mLog->toPlainText(), filter, this, KUrl(), i18n("Save Log"));
}

void StorageServiceLogDialog::setLog(const QString &log)
{
    mLog->setHtml(log);
}

void StorageServiceLogDialog::writeConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    KConfigGroup group = config->group( QLatin1String("StorageServiceLogDialog") );
    group.writeEntry( "Size", size() );
}

void StorageServiceLogDialog::readConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "StorageServiceLogDialog" );
    const QSize size = group.readEntry( "Size", QSize(600, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}
