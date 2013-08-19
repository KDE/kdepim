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
#ifndef MESSAGECOMPOSER_RECIPIENTSEDITORNG_H
#define MESSAGECOMPOSER_RECIPIENTSEDITORNG_H

#include "messagecomposer_export.h"

#include "recipientline.h"

#include <libkdepim/multiplyingline/multiplyinglineeditor.h>

namespace KMime {
namespace Types {
class Mailbox;
}
}
namespace MessageComposer {

class MESSAGECOMPOSER_EXPORT RecipientLineFactory : public KPIM::MultiplyingLineFactory
{
    Q_OBJECT
public:
    explicit RecipientLineFactory( QObject* parent ) : KPIM::MultiplyingLineFactory( parent ) {}
    virtual KPIM::MultiplyingLine* newLine(  QWidget *parent )
    {
        return new RecipientLineNG( parent );
    }
};


class RecipientsPicker;
class RecipientsEditorSideWidget;

class MESSAGECOMPOSER_EXPORT RecipientsEditor : public KPIM::MultiplyingLineEditor
{
    Q_OBJECT
public:
    explicit RecipientsEditor( QWidget *parent = 0 );
    ~RecipientsEditor();

    Recipient::List recipients() const;
    QSharedPointer<Recipient> activeRecipient() const;
    
    MessageComposer::RecipientsPicker* picker() const;

    void setRecipientString( const QList<KMime::Types::Mailbox> &mailboxes, Recipient::Type );
    QString recipientString( Recipient::Type ) const;
    QStringList recipientStringList( Recipient::Type ) const;

    /** Adds a recipient (or multiple recipients) to one line of the editor.
        @param recipient The recipient(s) you want to add.
        @param type      The recipient type.
    */
    void addRecipient( const QString & recipient, Recipient::Type type );

    /** Removes the recipient provided it can be found and has the given type.
        @param recipient The recipient(s) you want to remove.
        @param type      The recipient type.
    */
    void removeRecipient( const QString & recipient, Recipient::Type type );

    /**
     * Sets the config file used for storing recent addresses.
     */
    void setRecentAddressConfig( KConfig *config );

public slots:
    void selectRecipients();
    void saveDistributionList();

protected slots:
    void slotPickedRecipient( const Recipient & );
    void slotLineAdded( KPIM::MultiplyingLine* );
    void slotLineDeleted( int pos );
    void slotCalculateTotal();

protected:
    virtual RecipientLineNG* activeLine() const;

private:
    KConfig *mRecentAddressConfig;
    RecipientsEditorSideWidget* mSideWidget;
};

}

#endif //MESSAGECOMPOSER_RECIPIENTSEDITORNG_H
