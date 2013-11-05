/*
  Copyright 2009 Olivier Trichet <nive@nivalis.org>

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include "identity_widget.h"

#include "identity_edition_dialog.h"
#include "kngroup.h"
#include "knnntpaccount.h"
#include "settings.h"
#include "settings_container_interface.h"

#include <KPIMIdentities/Identity>
#include <KPIMIdentities/IdentityManager>
#include <QPointer>


using namespace KPIMIdentities;

namespace KNode {

IdentityWidget::IdentityWidget( SettingsContainerInterface *settingsContainer, const KComponentData &inst, QWidget *parent )
  : KCModule( inst, parent ),
    mConfigurationContainer( settingsContainer )
{
  setupUi( this );
  if ( dynamic_cast< Settings * >( mConfigurationContainer ) ) {
    // Ensure the identity selector is not disabled
    mUseSpecificIdentity->setChecked( true );
    mUseSpecificIdentity->hide();
  }

  connect( mIdentitySelector, SIGNAL(identityChanged(uint)),
           this, SLOT(identitySelected(uint)) );

  connect( mUseSpecificIdentity, SIGNAL(clicked(bool)),
           this, SLOT(useSpecificIdentity(bool)) );

  connect( mModifyIdentitiesButton, SIGNAL(clicked(bool)),
           this, SLOT(modifyIdentities()) );
}

IdentityWidget::~IdentityWidget()
{
}


void IdentityWidget::load()
{
  const Identity &identity = mConfigurationContainer->identity();
  if ( identity.isNull() ) {
    mUseSpecificIdentity->setChecked( false );
    useSpecificIdentity( false );
  } else {
    mIdentitySelector->setCurrentIdentity( identity.uoid() );
    mUseSpecificIdentity->setChecked( true );
    useSpecificIdentity( true );
  }
}

void IdentityWidget::save()
{
  if ( mUseSpecificIdentity->isChecked() ) {
    IdentityManager *im = KNGlobals::self()->identityManager();
    uint uoid = mIdentitySelector->currentIdentity();
    mConfigurationContainer->setIdentity( im->identityForUoid( uoid ) );
  } else {
    mConfigurationContainer->setIdentity( Identity::null() );
  }

  mConfigurationContainer->writeConfig();
}


void IdentityWidget::identitySelected( uint uoid )
{
  KPIMIdentities::IdentityManager *im = KNGlobals::self()->identityManager();
  const Identity &identity = im->identityForUoid( uoid );
  loadFromIdentity( identity );

  emit changed( ( uoid != mConfigurationContainer->identity().uoid() ) );
}

void IdentityWidget::useSpecificIdentity( bool useSpecific )
{
  mIdentitySelector->setEnabled( useSpecific );
  mModifyIdentitiesButton->setEnabled( useSpecific );
  if ( useSpecific ) {
    identitySelected( mIdentitySelector->currentIdentity() );
  } else {
    loadFromIdentity( Identity::null() );
    emit changed( !mConfigurationContainer->identity().isNull() );
  }
}

void IdentityWidget::loadFromIdentity( const Identity &identity )
{
  mName->setText( identity.fullName() );
  mOrganisation->setText( identity.organization() );
  mEmail->setText( identity.primaryEmailAddress() );
  mReplyto->setText( identity.replyToAddr() );
  mMailcopiesto->setText( identity.property( "Mail-Copies-To" ).toString() );
}

void IdentityWidget::modifyIdentities()
{
  IdentityEditionDialog dlg( mIdentitySelector->currentIdentity(), this );
  dlg.exec();
  // Note: the combo box "mIdentitySelector" is updated automatically via
  // DBus through the IdentityManager

  // Reload information of the selected identity
  identitySelected( mIdentitySelector->currentIdentity() );
}

} // namespace KNode

