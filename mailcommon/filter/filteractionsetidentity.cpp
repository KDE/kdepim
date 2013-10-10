/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "filteractionsetidentity.h"
#include "messagecore/utils/stringutil.h"

#include "kernel/mailkernel.h"
#include "dialog/filteractionmissingargumentdialog.h"

#include <KDE/KPIMIdentities/Identity>
#include <KDE/KPIMIdentities/IdentityCombo>
#include <KDE/KPIMIdentities/IdentityManager>

#include <KDE/KLocale>
#include <QPointer>

using namespace MailCommon;

FilterAction* FilterActionSetIdentity::newAction()
{
    return new FilterActionSetIdentity;
}

FilterActionSetIdentity::FilterActionSetIdentity( QObject *parent )
    : FilterActionWithUOID( QLatin1String("set identity"), i18n( "Set Identity To" ), parent )
{
    mParameter = KernelIf->identityManager()->defaultIdentity().uoid();
}

bool FilterActionSetIdentity::argsFromStringInteractive( const QString &argsStr, const QString &filterName )
{
    bool needUpdate = false;
    argsFromString( argsStr );
    if ( KernelIf->identityManager()->identityForUoid( mParameter ).isNull() )
    {
        QPointer<FilterActionMissingIdentityDialog> dlg = new FilterActionMissingIdentityDialog( filterName );
        if ( dlg->exec() ) {
            mParameter = dlg->selectedIdentity();
            needUpdate = true;
        }
        else
            mParameter = -1;
        delete dlg;
    }
    return needUpdate;
}

FilterAction::ReturnCode FilterActionSetIdentity::process(ItemContext &context , bool applyOnOutbound) const
{
    const KPIMIdentities::Identity & ident =
      KernelIf->identityManager()->identityForUoid( mParameter );

    if ( ident.isNull() )
        return ErrorButGoOn;

    const KMime::Message::Ptr msg = context.item().payload<KMime::Message::Ptr>();
    const uint currentId = msg->headerByType( "X-KMail-Identity" ) ? msg->headerByType( "X-KMail-Identity" )->asUnicodeString().trimmed().toUInt() : 0;
    if (currentId != mParameter) {
        KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-Identity", msg.get(), QString::number( mParameter ), "utf-8" );
        if (applyOnOutbound) {
            msg->from()->fromUnicodeString( ident.fullEmailAddr(), "utf-8" );
            if (!ident.bcc().isEmpty()) {
                const KMime::Types::Mailbox::List mailboxes = MessageCore::StringUtil::mailboxListFromUnicodeString( ident.bcc() );
                foreach ( const KMime::Types::Mailbox &mailbox, mailboxes )
                    msg->bcc()->addAddress( mailbox );
            }
        }
        msg->setHeader( header );
        msg->assemble();

        context.setNeedsPayloadStore();
    }

    return GoOn;
}

SearchRule::RequiredPart FilterActionSetIdentity::requiredPart() const
{
    return SearchRule::CompleteMessage;
}


QWidget* FilterActionSetIdentity::createParamWidget( QWidget *parent ) const
{
    KPIMIdentities::IdentityCombo *comboBox = new KPIMIdentities::IdentityCombo( KernelIf->identityManager(), parent );
    comboBox->setCurrentIdentity( mParameter );

    connect( comboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(filterActionModified()) );

    return comboBox;
}

void FilterActionSetIdentity::applyParamWidgetValue( QWidget *paramWidget )
{
    const KPIMIdentities::IdentityCombo *comboBox = dynamic_cast<KPIMIdentities::IdentityCombo*>( paramWidget );
    Q_ASSERT( comboBox );

    mParameter = comboBox->currentIdentity();
}

void FilterActionSetIdentity::clearParamWidget( QWidget *paramWidget ) const
{
    KPIMIdentities::IdentityCombo *comboBox = dynamic_cast<KPIMIdentities::IdentityCombo*>( paramWidget );
    Q_ASSERT( comboBox );

    comboBox->setCurrentIndex( 0 );
}

void FilterActionSetIdentity::setParamWidgetValue( QWidget *paramWidget ) const
{
    KPIMIdentities::IdentityCombo *comboBox = dynamic_cast<KPIMIdentities::IdentityCombo*>( paramWidget );
    Q_ASSERT( comboBox );

    comboBox->setCurrentIdentity( mParameter );
}



#include "filteractionsetidentity.moc"
