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

#ifndef IDENTITYCOMBOBOXACTIVITY_H
#define IDENTITYCOMBOBOXACTIVITY_H

#include "pimactivity_export.h"
#include <KComboBox>

namespace KPIMIdentities {
class Identity;
}

namespace PimActivity {
class IdentityComboboxActivityPrivate;
class IdentityManagerActivity;
class PIMACTIVITY_EXPORT IdentityComboboxActivity : public KComboBox
{
    Q_OBJECT
public:
    explicit IdentityComboboxActivity(IdentityManagerActivity *manager, QWidget *parent = 0);
    ~IdentityComboboxActivity();
    QString currentIdentityName() const;

    uint currentIdentity() const;
    void setCurrentIdentity( const QString &identityName );
    void setCurrentIdentity( const KPIMIdentities::Identity &identity );
    void setCurrentIdentity( uint uoid );

protected Q_SLOTS:
    void updateComboboxList(const QString &id);

private:
    friend class IdentityComboboxActivityPrivate;
    IdentityComboboxActivityPrivate * const d;
    Q_PRIVATE_SLOT( d, void slotCurrentActivityChanged(const QString&))
};
}

#endif // IDENTITYCOMBOBOXACTIVITY_H
