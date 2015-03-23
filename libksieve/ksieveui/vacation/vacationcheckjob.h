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

#ifndef VACATIONCHECKJOB_H
#define VACATIONCHECKJOB_H

#include <QObject>
#include <QStringList>
#include <KUrl>

namespace KManageSieve {
class SieveJob;
}

namespace KSieveUi {
class ParseUserScriptJob;
class VacationCheckJob : public QObject
{
    Q_OBJECT
public:
    explicit VacationCheckJob(const KUrl &url, const QString &serverName, QObject *parent=0);
    ~VacationCheckJob();
    void setKep14Support(bool kep14Support);
    void start();
    bool noScriptFound();
    QString script();
    QStringList sieveCapabilities();
    QString serverName();

Q_SIGNALS:
    void scriptActive(VacationCheckJob* job, const QString &sscriptName, bool active);

private slots:
    void slotGetResult(KManageSieve::SieveJob *job, bool success, const QString &script, bool active);
    void slotGotActiveScripts(ParseUserScriptJob *job);
    void slotGotList(KManageSieve::SieveJob *job, bool success, const QStringList &availableScripts, const QString &activeScript);
    void emitError(const QString &errorMessage);
    void searchVacationScript();
    void getNextScript();
    bool isVacationScipt(const QString &script) const;
    bool isLastScript() const;

private:
    QString mServerName;
    KUrl mUrl;
    KManageSieve::SieveJob * mSieveJob;
    ParseUserScriptJob *mParseJob;
    bool mKep14Support;
    QStringList mAvailableScripts;
    QStringList mActiveScripts;
    int mScriptPos;
    bool mNoScriptFound;
    QString mScript;
    QStringList mSieveCapabilities;
};
}

#endif // VACATIONCHECKJOB_H
