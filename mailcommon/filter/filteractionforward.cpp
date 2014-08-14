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

#include "filteractionforward.h"

#include "kernel/mailkernel.h"
#include "util/mailutil.h"
#include "dialog/filteractionmissingargumentdialog.h"

#include <pimcommon/widgets/minimumcombobox.h>
#include <messagecomposer/helper/messagefactory.h>
#include <messagecomposer/sender/messagesender.h>
#include <messagecore/widgets/emailaddressrequester.h>
#include <messagecore/utils/stringutil.h>
#include <templateparser/customtemplates.h>
#include <templateparser/customtemplates_kfg.h>

#include <QDebug>
#include <KLocalizedString>
#include <KLineEdit>

#include <QHBoxLayout>

using namespace MailCommon;

FilterAction *FilterActionForward::newAction()
{
    return new FilterActionForward;
}

FilterActionForward::FilterActionForward( QObject *parent )
    : FilterActionWithAddress(QLatin1String( "forward"), i18nc( "Forward directly not with a command", "Forward To" ), parent )
{
}

FilterAction::ReturnCode FilterActionForward::process(ItemContext &context , bool) const
{
    if ( mParameter.isEmpty() )
        return ErrorButGoOn;

    const KMime::Message::Ptr msg = context.item().payload<KMime::Message::Ptr>();
    // avoid endless loops when this action is used in a filter
    // which applies to sent messages
    if ( MessageCore::StringUtil::addressIsInAddressList( mParameter,
                                                          QStringList( msg->to()->asUnicodeString() ) ) ) {
        qWarning() << "Attempt to forward to receipient of original message, ignoring.";
        return ErrorButGoOn;
    }

    MessageComposer::MessageFactory factory( msg, context.item().id() );
    factory.setIdentityManager( KernelIf->identityManager() );
    factory.setFolderIdentity( Util::folderIdentity( context.item() ) );
    factory.setTemplate( mTemplate );

    KMime::Message::Ptr fwdMsg = factory.createForward();
    fwdMsg->to()->fromUnicodeString( fwdMsg->to()->asUnicodeString() + QLatin1Char( ',' ) + mParameter, "utf-8" );
    if ( !KernelIf->msgSender()->send( fwdMsg, MessageComposer::MessageSender::SendDefault ) ) {
        qWarning() << "FilterAction: could not forward message (sending failed)";
        return ErrorButGoOn; // error: couldn't send
    } else
        sendMDN( context.item(), KMime::MDN::Dispatched );

    // (the msgSender takes ownership of the message, so don't delete it here)
    return GoOn;
}

SearchRule::RequiredPart FilterActionForward::requiredPart() const
{
    return SearchRule::CompleteMessage;
}

QWidget* FilterActionForward::createParamWidget( QWidget *parent ) const
{
    QWidget *addressAndTemplate = new QWidget( parent );
    QHBoxLayout *layout = new QHBoxLayout( addressAndTemplate );
    layout->setMargin( 0 );

    QWidget *addressEdit = FilterActionWithAddress::createParamWidget( addressAndTemplate );
    addressEdit->setObjectName( QLatin1String("addressEdit") );
    layout->addWidget( addressEdit );

    MessageCore::EmailAddressRequester *addressRequester = qobject_cast<MessageCore::EmailAddressRequester*>( addressEdit );
    Q_ASSERT( addressRequester );
    KLineEdit *lineEdit = addressRequester->lineEdit();
    lineEdit->setClearButtonEnabled(true);
    lineEdit->setTrapReturnKey(true);
    lineEdit->setToolTip( i18n( "The addressee to whom the message will be forwarded." ) );
    lineEdit->setWhatsThis( i18n( "The filter will forward the message to the addressee entered here." ) );

    PimCommon::MinimumComboBox *templateCombo = new PimCommon::MinimumComboBox( addressAndTemplate );
    templateCombo->setObjectName( QLatin1String("templateCombo") );
    layout->addWidget( templateCombo );

    templateCombo->addItem( i18n( "Default Template" ) );

    const QStringList templateNames = SettingsIf->customTemplates();
    foreach( const QString &templateName, templateNames ) {
        TemplateParser::CTemplates templat( templateName );
        if ( templat.type() == TemplateParser::CustomTemplates::TForward ||
             templat.type() == TemplateParser::CustomTemplates::TUniversal ) {
            templateCombo->addItem( templateName );
        }
    }

    templateCombo->setEnabled( templateCombo->count() > 1 );
    templateCombo->setToolTip( i18n( "The template used when forwarding" ) );
    templateCombo->setWhatsThis( i18n( "Set the forwarding template that will be used with this filter." ) );

    connect( templateCombo, SIGNAL(currentIndexChanged(int)),
             this, SIGNAL(filterActionModified()) );
    connect( addressRequester, SIGNAL(textChanged()),
             this, SIGNAL(filterActionModified()) );

    return addressAndTemplate;
}

void FilterActionForward::applyParamWidgetValue( QWidget *paramWidget )
{
    QWidget *addressEdit = paramWidget->findChild<QWidget*>( QLatin1String("addressEdit") );
    Q_ASSERT( addressEdit );
    FilterActionWithAddress::applyParamWidgetValue( addressEdit );

    const PimCommon::MinimumComboBox *templateCombo = paramWidget->findChild<PimCommon::MinimumComboBox*>( QLatin1String("templateCombo") );
    Q_ASSERT( templateCombo );

    if ( templateCombo->currentIndex() == 0 ) {
        // Default template, so don't use a custom one
        mTemplate.clear();
    } else {
        mTemplate = templateCombo->currentText();
    }
}

void FilterActionForward::setParamWidgetValue( QWidget *paramWidget ) const
{
    QWidget *addressEdit = paramWidget->findChild<QWidget*>( QLatin1String("addressEdit") );
    Q_ASSERT( addressEdit );
    FilterActionWithAddress::setParamWidgetValue( addressEdit );

    PimCommon::MinimumComboBox *templateCombo = paramWidget->findChild<PimCommon::MinimumComboBox*>( QLatin1String("templateCombo") );
    Q_ASSERT( templateCombo );

    if ( mTemplate.isEmpty() ) {
        templateCombo->setCurrentIndex( 0 );
    } else {
        int templateIndex = templateCombo->findText( mTemplate );
        if ( templateIndex != -1 ) {
            templateCombo->setCurrentIndex( templateIndex );
        } else {
            mTemplate.clear();
        }
    }
}

void FilterActionForward::clearParamWidget( QWidget *paramWidget ) const
{
    QWidget *addressEdit = paramWidget->findChild<QWidget*>( QLatin1String("addressEdit") );
    Q_ASSERT( addressEdit );
    FilterActionWithAddress::clearParamWidget( addressEdit );

    PimCommon::MinimumComboBox *templateCombo = paramWidget->findChild<PimCommon::MinimumComboBox*>( QLatin1String("templateCombo") );
    Q_ASSERT( templateCombo );

    templateCombo->setCurrentIndex( 0 );
}

// We simply place a "@$$@" between the two parameters. The template is the last
// parameter in the string, for compatibility reasons.
static const QString forwardFilterArgsSeperator = QLatin1String("@$$@");

void FilterActionForward::argsFromString( const QString &argsStr )
{
    const int seperatorPos = argsStr.indexOf( forwardFilterArgsSeperator );

    if ( seperatorPos == - 1 ) {
        // Old config, assume that the whole string is the addressee
        FilterActionWithAddress::argsFromString( argsStr );
    } else {
        const QString addressee = argsStr.left( seperatorPos );
        mTemplate = argsStr.mid( seperatorPos + forwardFilterArgsSeperator.length() );
        FilterActionWithAddress::argsFromString( addressee );
    }
}

bool FilterActionForward::argsFromStringInteractive( const QString &argsStr, const QString& filterName )
{
    bool needUpdate = false;
    argsFromString( argsStr );
    if ( !mTemplate.isEmpty() ) {
        const QStringList templateNames = SettingsIf->customTemplates();
        QStringList currentTemplateList;
        currentTemplateList << i18n( "Default Template" );
        foreach( const QString &templateName, templateNames ) {
            TemplateParser::CTemplates templat( templateName );
            if ( templat.type() == TemplateParser::CustomTemplates::TForward ||
                 templat.type() == TemplateParser::CustomTemplates::TUniversal ) {
                if ( templateName == mTemplate ) {
                    return false;
                }
                currentTemplateList << templateName;
            }
        }
        QPointer<FilterActionMissingTemplateDialog> dlg = new FilterActionMissingTemplateDialog( currentTemplateList, filterName );
        if ( dlg->exec() ) {
            mTemplate = dlg->selectedTemplate();
            needUpdate = true;
        }
        delete dlg;
    }
    return needUpdate;
}


QString FilterActionForward::argsAsString() const
{
    return FilterActionWithAddress::argsAsString() + forwardFilterArgsSeperator + mTemplate;
}

QString FilterActionForward::displayString() const
{
    if ( mTemplate.isEmpty() )
        return i18n( "Forward to %1 with default template", mParameter );
    else
        return i18n( "Forward to %1 with template %2", mParameter, mTemplate );
}


