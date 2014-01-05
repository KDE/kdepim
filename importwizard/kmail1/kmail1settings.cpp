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

#include "kmail1settings.h"
#include "importwizardutil.h"

#include <mailtransport/transportmanager.h>

#include <kpimidentities/identity.h>
#include <kpimidentities/signature.h>


KMail1Settings::KMail1Settings(const QString& filename,ImportWizard *parent)
  :AbstractSettings( parent )
{
    readImapAccount();
    readTransport();
    readIdentity();
    readGlobalSettings();
}

KMail1Settings::~KMail1Settings()
{
}

void KMail1Settings::readImapAccount()
{
}

void KMail1Settings::readTransport()
{
}

void KMail1Settings::readIdentity()
{
}


void KMail1Settings::readGlobalSettings()
{
    //TODO
}
