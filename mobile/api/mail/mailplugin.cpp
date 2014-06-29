/*
Copyright 2014  Michael Bohlender michael.bohlender@kdemail.net

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "mailplugin.h"

#include "folderlist.h"
#include "message.h"
#include "composer.h"
#include "error.h"
#include "receivermodel.h"

#include <qdeclarative.h>

void MailPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == "org.kde.pim.mail");
    qmlRegisterType<FolderList>(uri, 0, 1, "FolderList");
    qmlRegisterType<Message>(uri, 0, 1, "Message");
    qmlRegisterType<Composer>(uri, 0, 1, "Composer");
    qmlRegisterType<Error>(uri, 0, 1, "Error");
    qmlRegisterType<ReceiverModel>(uri, 0, 1, "ReceiverModel");
}

//QT5 Q_EXPORT_PLUGIN2(mailplugin, MailPlugin)
