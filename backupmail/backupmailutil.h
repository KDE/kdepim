/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#ifndef BACKUPUTIL_H
#define BACKUPUTIL_H
#include <Qt>
namespace BackupMailUtil {
  enum BackupType {
    None = 0,
    Identity = 1,
    Mails = 2,
    MailTransport = 4,
    Resources = 8,
    Config = 16,
    AkonadiDb = 32,
    Nepomuk = 64 //TODO
  };
  Q_DECLARE_FLAGS(BackupTypes, BackupType )

  static QString transportsPath()
  {
    return QLatin1String("transports/");
  }

  static QString resourcesPath()
  {
    return QLatin1String("resources/");
  }

  static QString identitiesPath()
  {
    return QLatin1String("identities/");
  }

  static QString mailsPath()
  {
    return QLatin1String("mails/");
  }

  static QString configsPath()
  {
    return QLatin1String("configs/");
  }

  static QString akonadiPath()
  {
    return QLatin1String("akonadi/");
  }

}
#endif // UTIL_H
