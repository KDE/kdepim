
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
    prof.setManParts( m_choo->choosen() );
    return prof;
}

#include "profilewizard.moc"
