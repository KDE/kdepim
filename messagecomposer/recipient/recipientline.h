/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
    Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#ifndef RECIPIENTLINE_H
#define RECIPIENTLINE_H

#include <QSharedPointer>

#include "messagecomposer/recipient/recipient.h"

#include <libkdepim/multiplyingline/multiplyingline.h>
#include <messagecomposer/composer/composerlineedit.h>

#include <KComboBox>

namespace MessageComposer {

class RecipientComboBox : public KComboBox
{
    Q_OBJECT
public:
    explicit RecipientComboBox( QWidget *parent );

signals:
    void rightPressed();

protected:
    void keyPressEvent( QKeyEvent *ev );
};

class RecipientLineEdit : public MessageComposer::ComposerLineEdit
{
    Q_OBJECT
public:
    explicit RecipientLineEdit( QWidget * parent );

signals:
    void deleteMe();
    void leftPressed();
    void rightPressed();

protected:
    void keyPressEvent( QKeyEvent *ev );
};

class RecipientLineNG : public KPIM::MultiplyingLine
{
    Q_OBJECT
public:
    explicit RecipientLineNG( QWidget* parent );
    virtual ~RecipientLineNG(){}

    virtual void activate();
    virtual bool isActive() const;

    virtual bool isEmpty() const;
    virtual void clear();
    
    virtual bool isModified() const;
    virtual void clearModified();

    virtual KPIM::MultiplyingLineData::Ptr data() const;
    virtual void setData( const KPIM::MultiplyingLineData::Ptr &data );

    virtual void fixTabOrder( QWidget* previous );
    virtual QWidget* tabOut() const;
    
    virtual void moveCompletionPopup();
    virtual void setCompletionMode( KGlobalSettings::Completion mode );
    
    virtual int setColumnWidth( int w );

    // recipient specific methods
    int recipientsCount() const;

    void setRecipientType( Recipient::Type );
    Recipient::Type recipientType() const;
    QSharedPointer<Recipient> recipient() const;

    /**
     * Sets the config file used for storing recent addresses.
     */
    void setRecentAddressConfig( KConfig *config );

signals:
    void typeModified( RecipientLineNG* );
    void countChanged();

protected slots:
    void slotEditingFinished();
    void slotTypeModified();
    void analyzeLine( const QString & );

private:
    void dataFromFields();
    void fieldsFromData();
    RecipientComboBox *mCombo;
    RecipientLineEdit *mEdit;
    int mRecipientsCount;
    bool mModified;
    QSharedPointer<Recipient> mData;

};

}

#endif // RECIPIENTLINE_H
