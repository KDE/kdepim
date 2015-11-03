/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "modifymessagedisplayformatjob.h"
#include "messageviewer_debug.h"

#include <AkonadiCore/ItemModifyJob>

#include "viewer/messagedisplayformatattribute.h"

using namespace MessageViewer;
ModifyMessageDisplayFormatJob::ModifyMessageDisplayFormatJob(QObject *parent)
    : QObject(parent),
      mMessageFormat(Viewer::UseGlobalSetting),
      mRemoteContent(false),
      mResetFormat(false)
{

}

ModifyMessageDisplayFormatJob::~ModifyMessageDisplayFormatJob()
{

}

void ModifyMessageDisplayFormatJob::setRemoteContent(bool remote)
{
    mRemoteContent = remote;
}

void ModifyMessageDisplayFormatJob::setMessageFormat(Viewer::DisplayFormatMessage format)
{
    mMessageFormat = format;
}

void ModifyMessageDisplayFormatJob::setResetFormat(bool resetFormat)
{
    mResetFormat = resetFormat;
}

void ModifyMessageDisplayFormatJob::start()
{
    if (mMessageItem.isValid()) {
        if (mResetFormat)  {
            resetDisplayFormat();
        } else {
            modifyDisplayFormat();
        }
    } else {
        qCDebug(MESSAGEVIEWER_LOG) << " messageItem doesn't exist";
        deleteLater();
    }
}

void ModifyMessageDisplayFormatJob::setMessageItem(const Akonadi::Item &messageItem)
{
    mMessageItem = messageItem;
}

void ModifyMessageDisplayFormatJob::resetDisplayFormat()
{
    mMessageItem.removeAttribute<MessageViewer::MessageDisplayFormatAttribute>();
    Akonadi::ItemModifyJob *modify = new Akonadi::ItemModifyJob(mMessageItem);
    modify->setIgnorePayload(true);
    modify->disableRevisionCheck();
    connect(modify, &KJob::result, this, &ModifyMessageDisplayFormatJob::slotModifyItemDone);
}

void ModifyMessageDisplayFormatJob::modifyDisplayFormat()
{
    MessageViewer::MessageDisplayFormatAttribute *attr  = mMessageItem.attribute<MessageViewer::MessageDisplayFormatAttribute>(Akonadi::Item::AddIfMissing);
    attr->setRemoteContent(mRemoteContent);
    attr->setMessageFormat(mMessageFormat);
    Akonadi::ItemModifyJob *modify = new Akonadi::ItemModifyJob(mMessageItem);
    modify->setIgnorePayload(true);
    modify->disableRevisionCheck();
    connect(modify, &KJob::result, this, &ModifyMessageDisplayFormatJob::slotModifyItemDone);
}

void ModifyMessageDisplayFormatJob::slotModifyItemDone(KJob *job)
{
    if (job && job->error()) {
        qCWarning(MESSAGEVIEWER_LOG) << " Error trying to change attribute:" << job->errorText();
    }
    deleteLater();
}
