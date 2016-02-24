/*
  Copyright (c) 2016 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/
#ifndef ACLMODIFYJOB_H
#define ACLMODIFYJOB_H

#include <QObject>
#include <KIMAP/Acl>
#include <akonadi/collection.h>
class KJob;
namespace PimCommon
{
class ImapAclAttribute;
class AclModifyJob : public QObject
{
    Q_OBJECT
public:
    explicit AclModifyJob(QObject *parent = 0);
    ~AclModifyJob();

    void setTopLevelCollection(const Akonadi::Collection &topLevelCollection);
    void setRecursive(bool recursive);
    void setNewRights(const QMap<QByteArray, KIMAP::Acl::Rights> &right);
    void start();
private Q_SLOTS:
    void slotModifyDone(KJob *job);
    void slotFetchCollectionFinished(const Akonadi::Collection::List &collectionList);
    void slotFetchCollectionFailed();
private:
    void changeAcl(Akonadi::Collection collection);
    void checkNewCollection();
    bool canAdministrate(PimCommon::ImapAclAttribute *attribute, const Akonadi::Collection &collection) const;
    Akonadi::Collection mTopLevelCollection;
    Akonadi::Collection::List mRecursiveCollection;
    QMap<QByteArray, KIMAP::Acl::Rights> mNewRight;
    bool mRecursive;
    int mCurrentIndex;
};
}
#endif // ACLMODIFYJOB_H
