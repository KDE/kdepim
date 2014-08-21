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

#include "storageservicewarning.h"
#include "storageservicelogdialog.h"

#include <KLocalizedString>
#include <QDateTime>
#include <QLocale>
#include <QPointer>

StorageServiceWarning::StorageServiceWarning(QWidget *parent)
    : KMessageWidget(parent)
{
    setVisible(false);
    setCloseButtonVisible(true);
    setMessageType(Error);
    setWordWrap(true);
    setText(i18n("Actions failed. <a href=\"actionfailed\">(Details...)</a>"));
    connect(this, SIGNAL(linkActivated(QString)), SLOT(slotShowDetails(QString)));
}

StorageServiceWarning::~StorageServiceWarning()
{

}

void StorageServiceWarning::slotShowDetails(const QString &content)
{
    if (content == QLatin1String("actionfailed")) {
        showLog();
    }
}

void StorageServiceWarning::addLog(const QString &log)
{
    const QString dateTime = QString::fromLatin1("<b>[%1] </b>").arg(QLocale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat));
    mLogInformation.append(dateTime + log + QLatin1String("<br>"));
}

void StorageServiceWarning::slotClearLog()
{
    mLogInformation.clear();
    animatedHide();
}

void StorageServiceWarning::showLog()
{
    QPointer<StorageServiceLogDialog> dlg = new StorageServiceLogDialog(this);
    connect(dlg, SIGNAL(clearLog()), this, SLOT(slotClearLog()));
    dlg->setLog(mLogInformation);
    dlg->exec();
    delete dlg;
}
