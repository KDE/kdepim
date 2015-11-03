/*
  Copyright (C) 2014 Christian Mollekopf <mollekopf@kolabsys.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef KDEPIM_PERSONSEARCHJOB_H
#define KDEPIM_PERSONSEARCHJOB_H

#include "kdepim_export.h"

#include <KJob>
#include <AkonadiCore/Collection>
#include <libkdepim/ldapclientsearch.h>
#include "person.h"

namespace KPIM
{
class PersonSearchJobPrivate;
class KDEPIM_EXPORT PersonSearchJob : public KJob
{
    Q_OBJECT
public:
    explicit PersonSearchJob(const QString &searchString, QObject *parent = Q_NULLPTR);
    virtual ~PersonSearchJob();

    void start() Q_DECL_OVERRIDE;

    QList<Person> matches() const;

Q_SIGNALS:
    void personsFound(const QVector<Person> &persons);
    void personUpdate(const Person &person);

public Q_SLOTS:
    bool kill(KillVerbosity verbosity = Quietly);

private Q_SLOTS:
    void onCollectionsReceived(const Akonadi::Collection::List &);
    void onCollectionsFetched(KJob *);
    void onLDAPSearchData(const KLDAP::LdapResultObject::List &);
    void onLDAPSearchDone();
    void updatePersonCollection(const Person &person);
    void modifyResult(KJob *job);

private:
    PersonSearchJobPrivate *const d;
};

}

#endif
