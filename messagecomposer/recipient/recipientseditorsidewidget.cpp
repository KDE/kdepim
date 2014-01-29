/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
    Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    Refactored from earlier code by:
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "recipientseditorsidewidget.h"

#include "recipientspicker.h"
#include "kwindowpositioner.h"

#include <KDialog>
#include <KLocalizedString>
#include <KPushButton>

#include <QLabel>
#include <QBoxLayout>
#include <QTextDocument>

using namespace MessageComposer;

RecipientsEditorSideWidget::RecipientsEditorSideWidget( RecipientsEditor *view, QWidget *parent )
    : QWidget( parent ), mEditor( view ), mRecipientPicker( 0 ), mPickerPositioner( 0 )
{
    QBoxLayout *topLayout = new QVBoxLayout( this );

    topLayout->setSpacing( KDialog::spacingHint() );
    topLayout->setMargin( 0 );
    topLayout->addStretch( 1 );

    mTotalLabel = new QLabel( this );
    mTotalLabel->setAlignment( Qt::AlignCenter );
    topLayout->addWidget( mTotalLabel );
    mTotalLabel->hide();

    topLayout->addStretch( 1 );

    mDistributionListButton = new KPushButton(
                i18nc("@action:button","Save List..."), this );
    topLayout->addWidget( mDistributionListButton );
    mDistributionListButton->hide();
    connect( mDistributionListButton, SIGNAL(clicked()),
             SIGNAL(saveDistributionList()) );
    mDistributionListButton->setToolTip(
                i18nc( "@info:tooltip", "Save recipients as distribution list") );

    mSelectButton = new QPushButton(
                i18nc( "@action:button Open recipient selection dialog.", "Se&lect..."), this );
    topLayout->addWidget( mSelectButton );
    connect( mSelectButton, SIGNAL(clicked()), SLOT(pickRecipient()) );
    mSelectButton->setToolTip( i18nc("@info:tooltip","Select recipients from address book") );
    updateTotalToolTip();
}

RecipientsEditorSideWidget::~RecipientsEditorSideWidget()
{
}


RecipientsPicker* RecipientsEditorSideWidget::picker() const
{
    if ( !mRecipientPicker ) {
        // hacks to allow picker() to be const in the presence of lazy loading
        RecipientsEditorSideWidget *non_const_this = const_cast<RecipientsEditorSideWidget*>( this );
        mRecipientPicker = new RecipientsPicker( non_const_this );
        connect( mRecipientPicker, SIGNAL(pickedRecipient(Recipient)),
                 non_const_this, SIGNAL(pickedRecipient(Recipient)) );
        mPickerPositioner = new KWindowPositioner( mSelectButton, mRecipientPicker );
    }
    return mRecipientPicker;
}

void RecipientsEditorSideWidget::setFocus()
{
    mSelectButton->setFocus();
}

void RecipientsEditorSideWidget::setTotal( int recipients, int lines )
{
    QString labelText;
    if ( recipients == 0 ) labelText = i18nc("@info:status No recipients selected"
                                             , "No recipients");
    else labelText = i18ncp("@info:status Number of recipients selected"
                            , "1 recipient","%1 recipients", recipients );
    mTotalLabel->setText( labelText );

    if ( lines > 3 ) mTotalLabel->show();
    else mTotalLabel->hide();

    if ( lines > 2 ) mDistributionListButton->show();
    else mDistributionListButton->hide();

    updateTotalToolTip();
}

void RecipientsEditorSideWidget::updateTotalToolTip()
{
    QString text = QLatin1String( "<qt>" );

    QString to;
    QString cc;
    QString bcc;

    Recipient::List recipients = mEditor->recipients();
    Recipient::List::ConstIterator it;
    for( it = recipients.constBegin(); it != recipients.constEnd(); ++it ) {
        QString emailLine = QLatin1String("&nbsp;&nbsp;") + Qt::escape( (*it)->email() ) + QLatin1String("<br/>");
        switch( (*it)->type() ) {
        case Recipient::To:
            to += emailLine;
            break;
        case Recipient::Cc:
            cc += emailLine;
            break;
        case Recipient::Bcc:
            bcc += emailLine;
            break;
        default:
            break;
        }
    }

    text += i18nc("@info:tooltip %1 list of emails", "<interface>To:</interface><nl/>%1", to);
    if ( !cc.isEmpty() ) {
        text += i18nc("@info:tooltip %1 list of emails", "<interface>CC:</interface><nl/>%1", cc);
    }
    if ( !bcc.isEmpty() ) {
        text += i18nc("@info:tooltip %1 list of emails", "<interface>BCC:</interface><nl/>%1", bcc);
    }

    text.append( QLatin1String("</qt>") );
    mTotalLabel->setToolTip( text );
}

void RecipientsEditorSideWidget::pickRecipient()
{
    MessageComposer::RecipientsPicker *p = picker();
    Recipient::Ptr rec = mEditor->activeRecipient();
    if( rec ) {
        p->setDefaultType( rec->type() );
        p->setRecipients( mEditor->recipients() );
        mPickerPositioner->reposition();
        p->show();
    }
}


#include "moc_recipientseditorsidewidget.cpp"
