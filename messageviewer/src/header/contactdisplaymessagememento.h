/* Copyright (C) 2012-2015 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CONTACTDISPLAYMESSAGEMEMENTO_H
#define CONTACTDISPLAYMESSAGEMEMENTO_H

#include "interfaces/bodypart.h"
#include "messageviewer/viewer.h"

#include <KContacts/Picture>
#include <KContacts/Addressee>

#include <QObject>
#include <QPointer>

class KJob;
namespace Gravatar
{
class GravatarResolvUrlJob;
}

namespace Akonadi
{
class ContactSearchJob;
}
namespace MessageViewer
{

class ContactDisplayMessageMemento : public QObject, public Interface::BodyPartMemento
{
    Q_OBJECT
public:
    explicit ContactDisplayMessageMemento(const QString &emailAddress);
    ~ContactDisplayMessageMemento();
    void processAddress(const KContacts::Addressee &addressee);
    bool allowToRemoteContent() const;
    KContacts::Picture photo() const;

    bool finished() const;

    void detach() Q_DECL_OVERRIDE;

    QPixmap gravatarPixmap() const;

    QImage imageFromUrl() const;

Q_SIGNALS:
    // TODO: Factor our update and detach into base class
    void update(MessageViewer::Viewer::UpdateMode);
    void changeDisplayMail(Viewer::DisplayFormatMessage displayAsHtml, bool remoteContent);

private Q_SLOTS:
    void slotSearchJobFinished(KJob *job);

    void slotGravatarResolvUrlFinished(Gravatar::GravatarResolvUrlJob *);
private:
    bool searchPhoto(const KContacts::AddresseeList &list);
    Viewer::DisplayFormatMessage mForceDisplayTo;
    KContacts::Picture mPhoto;
    QPixmap mGravatarPixmap;
    QImage mImageFromUrl;
    QString mEmailAddress;
    bool mFinished;
    bool mMailAllowToRemoteContent;
    QPointer<Akonadi::ContactSearchJob> mSearchJob;
};

}

#endif /* CONTACTDISPLAYMESSAGE_H */

