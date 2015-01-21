/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef GRANTLEEPRINT_H
#define GRANTLEEPRINT_H

#include <QObject>
#include <grantlee/templateloader.h>
#include <KContacts/Addressee>

namespace Grantlee {
class Engine;
}

namespace KABPrinting {
class GrantleePrint : public QObject
{
    Q_OBJECT
public:
    explicit GrantleePrint(const QString &themePath, QObject *parent = 0);
    ~GrantleePrint();
    QString contactsToHtml( const KContacts::Addressee::List &contacts );

private:
    QString mErrorMessage;
    Grantlee::Engine *mEngine;
    QSharedPointer<Grantlee::FileSystemTemplateLoader> mTemplateLoader;
    Grantlee::Template mSelfcontainedTemplate;
};
}
#endif // GRANTLEEPRINT_H
