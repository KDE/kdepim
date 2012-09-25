/* Copyright (C) 2012 Laurent Montel <montel@kde.org>
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
#include "viewer.h"

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
  explicit ContactDisplayMessageMemento( const QString &emailAddress );
  ~ContactDisplayMessageMemento();
  enum ForceDisplayTo {
    Unknown = 0,
    Text = 1,
    Html = 2
  };
  void processAddress( const KABC::Addressee& addressee );
  bool allowToRemoteContent() const;
  bool forceToHtml() const;
  bool forceToText() const;
  KABC::Picture photo() const;

  bool finished() const;

  void detach();

signals:
  // TODO: Factor our update and detach into base class
  void update( MessageViewer::Viewer::UpdateMode );

private Q_SLOTS:
  void slotSearchJobFinished( KJob *job );

private:
  bool mFinished;
  bool mMailAllowToRemoteContent;
  ForceDisplayTo mForceDisplayTo;
  KABC::Picture mPhoto;
};

}


#endif /* CONTACTDISPLAYMESSAGE_H */

