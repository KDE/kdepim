/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "filteractionmove.h"
#include "mailcommon/util/mailutil.h"

#include "kernel/mailkernel.h"

#include <KLocalizedString>

using namespace MailCommon;

FilterAction *FilterActionMove::newAction()
{
    return new FilterActionMove;
}

FilterActionMove::FilterActionMove(QObject *parent)
    : FilterActionWithFolder(QStringLiteral("transfer"), i18n("Move Into Folder"), parent)
{
}

FilterAction::ReturnCode FilterActionMove::process(ItemContext &context , bool) const
{
    if (!mFolder.isValid()) {
        const Akonadi::Collection targetFolder = CommonKernel->collectionFromId(mFolder.id());
        if (!targetFolder.isValid()) {
            return ErrorButGoOn;
        }

        context.setMoveTargetCollection(targetFolder);
    } else {
        context.setMoveTargetCollection(mFolder);
    }
    return GoOn;
}

SearchRule::RequiredPart FilterActionMove::requiredPart() const
{
    return SearchRule::Envelope;
}

QString FilterActionMove::sieveCode() const
{
    QString path;
    if (KernelIf->collectionModel()) {
        path = MailCommon::Util::fullCollectionPath(mFolder);
    } else {
        path = QString::number(mFolder.id());
    }
    const QString result = QStringLiteral("fileinto \"%1\";").arg(path);
    return result;
}

QStringList FilterActionMove::sieveRequires() const
{
    return QStringList() << QStringLiteral("fileinto");
}

QString FilterActionMove::informationAboutNotValidAction() const
{
    //KF5 add i18n
    return QString::fromLatin1("Folder destination was not defined.");
}

