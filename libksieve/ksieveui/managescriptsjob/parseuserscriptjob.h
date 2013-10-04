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

#ifndef PARSEUSERSCRIPTJOB_H
#define PARSEUSERSCRIPTJOB_H

#include <QObject>
#include <KUrl>
class QDomDocument;
class QDomElement;
namespace KManageSieve {
class SieveJob;
}

namespace KSieveUi {
class ParseUserScriptJob : public QObject
{
    Q_OBJECT
public:
    explicit ParseUserScriptJob(QObject *parent=0);
    ~ParseUserScriptJob();

    void start();

    void scriptUrl(const KUrl &url);

private Q_SLOTS:
    void slotGetResult( KManageSieve::SieveJob *, bool, const QString &, bool );

Q_SIGNALS:
    void success(const QStringList &activeScriptList);
    void error(const QString &msgError);

private:
    QString loadInclude(const QDomElement &element);
    QStringList parsescript(const QDomDocument &doc);
    KUrl mCurrentUrl;
};
}

#endif // PARSEUSERSCRIPTJOB_H
