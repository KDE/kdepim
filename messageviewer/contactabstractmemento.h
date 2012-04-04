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

#ifndef CONTACTABSTRACTMEMENTO_H
#define CONTACTABSTRACTMEMENTO_H

#include "interfaces/bodypart.h"
#include <KABC/Addressee>
#include "viewer.h"

#include <KUrl>

#include <QObject>

class KJob;

namespace Akonadi {
  class ContactSearchJob;
}

namespace MessageViewer {

class ContactAbstractMemento : public QObject, public Interface::BodyPartMemento
{
  Q_OBJECT
  public:
    ContactAbstractMemento( const QString &emailAddress );

    bool finished() const;

  virtual void detach();
  virtual void processAddress( const KABC::Addressee& addressee );
  
  signals:
    // TODO: Factor our update and detach into base class
    void update( MessageViewer::Viewer::UpdateMode );

  private slots:
    void slotSearchJobFinished( KJob *job );

  private:
    bool mFinished;
};

}

#endif /* CONTACTABSTRACTMEMENTO_H */

