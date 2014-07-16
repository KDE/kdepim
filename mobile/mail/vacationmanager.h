/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

#ifndef VACATIONMANAGER_H
#define VACATIONMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QPointer>

namespace KSieveUi {
class Vacation;
}

class KAction;
class KActionCollection;

/**
 * @short A class that encapsulates the handling of vacation sieve scripts.
 */
class VacationManager : public QObject
{
  Q_OBJECT

  Q_PROPERTY( bool activeVacationScriptAvailable READ activeVacationScriptAvailable NOTIFY vacationScriptActivityChanged )

  public:
    /**
     * Creates a new vacation manager.
     *
     * @param actionCollection The action collection to register the edit action at.
     * @param kernel The object that provide a bool askToGoOnline() slot.
     * @param parent The parent object.
     */
    explicit VacationManager( KActionCollection *actionCollection, QObject *kernel, QObject *parent = 0 );

    /**
     * Destroys the vacation manager.
     */
    ~VacationManager();

    /**
     * Returns whether an active vacation script is available on the server.
     */
    bool activeVacationScriptAvailable() const;

  public Q_SLOTS:
    /**
     * Opens the vacation script edit dialog.
     */
    void editVacation();

  Q_SIGNALS:
    /**
     * This signal is emitted whenever the activity of the vacation script has changed.
     */
    void vacationScriptActivityChanged();

  private Q_SLOTS:
    void updateVacationScriptActivity(bool , const QString &serverName=QString());

    void checkVacation();

  private:
    bool askToGoOnline() const;

    QPointer<KSieveUi::Vacation> mVacation;
    QObject *mKernel;
    bool mVacationScriptIsActive;
    KAction *mEditAction;
};

#endif
