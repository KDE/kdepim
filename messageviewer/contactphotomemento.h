/* Copyright 2010 Thomas McGuire <mcguire@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef CONTACTPHOTOMEMENTO_H
#define CONTACTPHOTOMEMENTO_H

#include "interfaces/bodypart.h"

#include "viewer.h"

#include <KABC/Picture>

#include <KUrl>

#include <QImage>
#include <QObject>

class KJob;

namespace Akonadi {
  class ContactSearchJob;
}

namespace MessageViewer {

class ContactPhotoMemento : public QObject, public Interface::BodyPartMemento
{
  Q_OBJECT
  public:
    ContactPhotoMemento( const QString &emailAddress );

    bool finished() const;
    KABC::Picture photo() const;

    virtual void detach();

  signals:
    // TODO: Factor our update and detach into base class
    void update( Viewer::UpdateMode );

  private slots:
    void slotSearchJobFinished( KJob *job );

  private:
    bool mFinished;
    KABC::Picture mPhoto;
};

}

#endif
