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

#ifndef GENERATEGLOBALSCRIPTJOB_H
#define GENERATEGLOBALSCRIPTJOB_H

#include <QObject>
#include <QStringList>
#include <KUrl>
namespace KManageSieve {
class SieveJob;
}
namespace KSieveUi {
class GenerateGlobalScriptJob : public QObject
{
    Q_OBJECT
public:
    explicit GenerateGlobalScriptJob(const KUrl &url, QObject *parent=0);
    ~GenerateGlobalScriptJob();

    void writeGlobalScripts();

    void addUserActiveScripts(const QStringList &lstScript);

Q_SIGNALS:
    void success();
    void error(const QString &msgError);

private Q_SLOTS:
    void slotPutMasterResult( KManageSieve::SieveJob *, bool success );
    void slotPutUserResult( KManageSieve::SieveJob *, bool success );

private:
    void disableAllOtherScripts();
    void writeMasterScript();
    void writeUserScript();
    QStringList mListUserActiveScripts;
    KUrl mCurrentUrl;
};
}

#endif // GENERATEGLOBALSCRIPTJOB_H
