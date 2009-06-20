/*
    Copyright (c) 2005 by Volker Krause <vkrause@kde.org>
    Copyright (c) 2005 by Florian Schröder <florian@deltatauchi.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef SLOXFOLDERMANAGER_H
#define SLOXFOLDERMANAGER_H

#include <QMap>
#include <QObject>

#include <kurl.h>

#include "slox_export.h"

namespace KIO {
class Job;
class DavJob;
}
class KJob;
class SloxBase;
class SloxFolder;

class KSLOX_EXPORT SloxFolderManager : public QObject
{
    Q_OBJECT
  public:
    SloxFolderManager( SloxBase *res, const KUrl &baseUrl );
    ~SloxFolderManager();

    QMap<QString, SloxFolder*> folders() const { return mFolders; }
    void requestFolders();

  signals:
    void foldersUpdated();

  protected:
    void readFolders();

    QString cacheFile() const;

  protected slots:
    void slotResult( KJob * );

  private:
    KIO::DavJob *mDownloadJob;
    KUrl mBaseUrl;
    QMap<QString, SloxFolder*> mFolders;
    SloxBase *mRes;
};

#endif
