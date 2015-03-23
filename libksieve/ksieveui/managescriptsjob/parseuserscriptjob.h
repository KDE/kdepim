/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef PARSEUSERSCRIPTJOB_H
#define PARSEUSERSCRIPTJOB_H

#include <QObject>
#include <QStringList>
#include <KUrl>
#include "ksieveui_export.h"
class QDomDocument;
class QDomElement;
namespace KManageSieve {
class SieveJob;
}

namespace KSieveUi {
class KSIEVEUI_EXPORT ParseUserScriptJob : public QObject
{
    Q_OBJECT
public:
    explicit ParseUserScriptJob(const KUrl &url,QObject *parent=0);
    ~ParseUserScriptJob();

    void start();

    KUrl scriptUrl() const;

    QStringList activeScriptList() const;
    QString error() const;

private Q_SLOTS:
    void slotGetResult( KManageSieve::SieveJob *, bool, const QString &, bool );

Q_SIGNALS:
    void finished(ParseUserScriptJob* job);

private:
    void emitSuccess(const QStringList &activeScriptList);
    void emitError(const QString &msgError);
    static QString loadInclude(const QDomElement &element);
    static QStringList extractActiveScript(const QDomDocument &doc);
    static QStringList parsescript(const QString &script, bool &result);
    KUrl mCurrentUrl;
    KManageSieve::SieveJob *mSieveJob;
    QStringList mActiveScripts;
    QString mError;
};
}

#endif // PARSEUSERSCRIPTJOB_H
