/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <klineedit.h>
#include <klocale.h>

#include "profilewizardintro.h"
#include "profilewizardchooserimpl.h"

#include "profilewizard.h"


using namespace KSync;

ProfileWizard::ProfileWizard( const Profile& prof,
                              const ManPartService::ValueList& lst )
    : KWizard( 0, "wiz", true ), m_prof( prof )
{
    m_useProf = true;
    initUI();
    initProf(lst);
}

ProfileWizard::ProfileWizard( const ManPartService::ValueList& lst )
    : KWizard( 0, "wiz", true )
{
    m_useProf = false;
    initUI();
    init( lst );
}

ProfileWizard::~ProfileWizard()
{
// all deleted for us
}

void ProfileWizard::initUI()
{
    m_intro = new ProfileWizardIntro( this );
    addPage( m_intro, i18n("Intro") );

    m_choo = new ProfileWizardChooserImpl();
    addPage( m_choo, i18n("Chooser") );
    setFinishEnabled( m_choo, true );
}

void ProfileWizard::init( const ManPartService::ValueList& lst )
{
    m_choo->init(lst);
}

void ProfileWizard::initProf( const ManPartService::ValueList& lst )
{
    m_intro->lneName->setText( m_prof.name() );
    m_choo->init( lst, m_prof.manParts() );
}

Profile ProfileWizard::profile() const
{
    Profile prof;
    if ( m_useProf )
        prof = m_prof;

    prof.setName( m_intro->lneName->text() );
    prof.setManParts( m_choo->chosen() );
    return prof;
}

#include "profilewizard.moc"
