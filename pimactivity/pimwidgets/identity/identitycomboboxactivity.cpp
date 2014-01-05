/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#include "identitycomboboxactivity.h"
#include "activitymanager.h"
#include "identitymanageractivity.h"

#include <KPIMIdentities/Identity>

namespace PimActivity {
class IdentityComboboxActivityPrivate
{
public:
    IdentityComboboxActivityPrivate(IdentityManagerActivity *manager, IdentityComboboxActivity *qq)
        : identityManagerActivity(manager),
          q(qq)
    {
    }

    void connectSignals()
    {
        if (identityManagerActivity->activityManager()) {
            q->connect(identityManagerActivity->activityManager(), SIGNAL(currentActivityChanged(QString)), q, SLOT(slotCurrentActivityChanged(QString)));
            initializeActivity();
        }
    }

    void initializeActivity()
    {
        slotCurrentActivityChanged(identityManagerActivity->activityManager()->currentActivity());
    }

    void slotCurrentActivityChanged(const QString &id)
    {
        //TODO
        q->updateComboboxList(id);
    }

    QList<uint> uoidList;
    IdentityManagerActivity *identityManagerActivity;
    IdentityComboboxActivity *q;
};


IdentityComboboxActivity::IdentityComboboxActivity(IdentityManagerActivity *manager, QWidget *parent)
    : KComboBox(parent), d(new IdentityComboboxActivityPrivate(manager, this))
{
    d->connectSignals();
}

IdentityComboboxActivity::~IdentityComboboxActivity()
{
    delete d;
}

void IdentityComboboxActivity::updateComboboxList(const QString &id)
{
    clear();
    if (id.isEmpty()) {
        //not activity => show all identity
    } else {
        //Show current identity from id.
    }
    //TODO
}

uint IdentityComboboxActivity::currentIdentity() const
{
    return d->uoidList[ currentIndex()];
}

void IdentityComboboxActivity::setCurrentIdentity( const QString &identityName )
{

}

void IdentityComboboxActivity::setCurrentIdentity( const KPIMIdentities::Identity &identity )
{
    setCurrentIdentity( identity.uoid() );
}

void IdentityComboboxActivity::setCurrentIdentity( uint uoid )
{

}

}

#include "moc_identitycomboboxactivity.cpp"
