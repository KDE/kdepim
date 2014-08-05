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

#include "filteractionsettransport.h"

#include "dialog/filteractionmissingargumentdialog.h"

#include <KLocale>
#include <MailTransport/Transport>
#include <MailTransport/TransportComboBox>
#include <MailTransport/TransportManager>

#include <QTextDocument>
#include <QPointer>

using namespace MailCommon;

FilterAction* FilterActionSetTransport::newAction()
{
    return new FilterActionSetTransport;
}

QWidget* FilterActionSetTransport::createParamWidget( QWidget *parent ) const
{
    MailTransport::TransportComboBox *transportCombobox = new MailTransport::TransportComboBox( parent );
    setParamWidgetValue( transportCombobox );

    connect( transportCombobox, SIGNAL(currentIndexChanged(int)),
             this, SIGNAL(filterActionModified()) );

    return transportCombobox;
}

FilterActionSetTransport::FilterActionSetTransport( QObject *parent )
    : FilterAction( QLatin1String("set transport"), i18n( "Set Transport To" ), parent ), mParameter( -1 )
{
}

bool FilterActionSetTransport::argsFromStringInteractive( const QString &argsStr, const QString &filterName )
{
    bool needUpdate = false;
    argsFromString( argsStr );
    if ( !MailTransport::TransportManager::self()->transportById( mParameter,false ) )
    {
        QPointer<FilterActionMissingTransportDialog> dlg = new FilterActionMissingTransportDialog( filterName );
        if ( dlg->exec() ) {
            mParameter = dlg->selectedTransport();
            needUpdate = true;
        }
        else
            mParameter = -1;
        delete dlg;
    }
    return needUpdate;
}


FilterAction::ReturnCode FilterActionSetTransport::process(ItemContext &context , bool) const
{
    if ( isEmpty() )
        return ErrorButGoOn;

    const MailTransport::Transport *transport = MailTransport::TransportManager::self()->transportById( mParameter );
    //Error if we don't have transport here.
    if ( !transport )
        return ErrorButGoOn;

    const KMime::Message::Ptr msg = context.item().payload<KMime::Message::Ptr>();
    KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-Transport", msg.get(), argsAsString(), "utf-8" );
    msg->setHeader( header );
    msg->assemble();

    context.setNeedsPayloadStore();

    return GoOn;
}

SearchRule::RequiredPart FilterActionSetTransport::requiredPart() const
{
    return SearchRule::CompleteMessage;
}


void FilterActionSetTransport::applyParamWidgetValue( QWidget *paramWidget )
{
    const MailTransport::TransportComboBox *comboBox = dynamic_cast<MailTransport::TransportComboBox*>( paramWidget );
    Q_ASSERT( comboBox );

    mParameter = comboBox->currentTransportId();
    mTransportName.clear();
}

void FilterActionSetTransport::clearParamWidget( QWidget *paramWidget ) const
{
    MailTransport::TransportComboBox *comboBox = dynamic_cast<MailTransport::TransportComboBox*>( paramWidget );
    Q_ASSERT( comboBox );

    comboBox->setCurrentIndex( 0 );
}

void FilterActionSetTransport::setParamWidgetValue( QWidget *paramWidget ) const
{
    MailTransport::TransportComboBox *comboBox = dynamic_cast<MailTransport::TransportComboBox*>( paramWidget );
    Q_ASSERT( comboBox );

    comboBox->setCurrentTransport( mParameter );
}


bool FilterActionSetTransport::isEmpty() const
{
    return (mParameter == -1);
}

void FilterActionSetTransport::argsFromString( const QString &argsStr )
{
    mParameter = argsStr.trimmed().toInt();
}

QString FilterActionSetTransport::argsAsString() const
{
    return QString::number( mParameter );
}

QString FilterActionSetTransport::displayString() const
{
    if ( mTransportName.isEmpty() ) {
        const QStringList listNames = MailTransport::TransportManager::self()->transportNames();
        const int index = MailTransport::TransportManager::self()->transportIds().indexOf( mParameter );
        if ( index != -1 ) {
            mTransportName = listNames.at( index );
        }
    }
    return label() + QLatin1String( " \"" ) + ( mTransportName.isEmpty() ? argsAsString().toHtmlEscaped() : mTransportName ) + QLatin1String( "\"" );
}

