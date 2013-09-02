/*
  Copyright 2013 Laurent Montel <montel@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef ADDEMAILDISPLAYJOB_H
#define ADDEMAILDISPLAYJOB_H

#include "kdepim_export.h"

#include <kjob.h>

namespace Akonadi {
  class Item;
}

namespace KPIM {

class KDEPIM_EXPORT AddEmailDiplayJob : public KJob
{
  Q_OBJECT

  public:
    AddEmailDiplayJob( const QString &email, QWidget *parentWidget, QObject *parent = 0 );

    ~AddEmailDiplayJob();
    void showAsHTML(bool html);
    void remoteContent(bool b);

    virtual void start();

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void slotSearchDone( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotAddContactDone( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotCollectionsFetched( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotResourceCreationDone( KJob* ) )    
    Q_PRIVATE_SLOT( d, void slotContactEditorError(const QString &) )
    Q_PRIVATE_SLOT( d, void contactStored( const Akonadi::Item & ) )

    //@endcond
};

}

#endif
