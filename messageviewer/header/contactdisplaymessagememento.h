/* Copyright (C) 2012, 2013 Laurent Montel <montel@kde.org>
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
#include "viewer/viewer.h"

#include <KABC/Picture>
#include <KABC/Addressee>

#include <QObject>

class KJob;

namespace MessageViewer
{

class ContactDisplayMessageMemento : public QObject, public Interface::BodyPartMemento
{
    Q_OBJECT
public:
    explicit ContactDisplayMessageMemento(const QString &emailAddress);
    ~ContactDisplayMessageMemento();
    void processAddress(const KABC::Addressee &addressee);
    bool allowToRemoteContent() const;
    KABC::Picture photo() const;

    bool finished() const;

    void detach();

signals:
    // TODO: Factor our update and detach into base class
    void update(MessageViewer::Viewer::UpdateMode);
    void changeDisplayMail(Viewer::DisplayFormatMessage displayAsHtml, bool remoteContent);

private Q_SLOTS:
    void slotSearchJobFinished(KJob *job);

private:
    void searchPhoto(const KABC::AddresseeList &list);
    bool mFinished;
    bool mMailAllowToRemoteContent;
    Viewer::DisplayFormatMessage mForceDisplayTo;
    KABC::Picture mPhoto;
};

}

#endif /* CONTACTDISPLAYMESSAGE_H */

