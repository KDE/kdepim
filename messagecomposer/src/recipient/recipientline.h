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

#include "MessageComposer/Recipient"

#include <Libkdepim/MultiplyingLine>
#include <MessageComposer/ComposerLineEdit>

#include <KComboBox>

namespace MessageComposer
{

class RecipientComboBox : public KComboBox
{
    Q_OBJECT
public:
    explicit RecipientComboBox(QWidget *parent);

Q_SIGNALS:
    void rightPressed();

protected:
    void keyPressEvent(QKeyEvent *ev) Q_DECL_OVERRIDE;
};

class RecipientLineEdit : public MessageComposer::ComposerLineEdit
{
    Q_OBJECT
public:
    explicit RecipientLineEdit(QWidget *parent);

Q_SIGNALS:
    void deleteMe();
    void leftPressed();
    void rightPressed();

protected:
    void keyPressEvent(QKeyEvent *ev) Q_DECL_OVERRIDE;
};

class RecipientLineNG : public KPIM::MultiplyingLine
{
    Q_OBJECT
public:
    explicit RecipientLineNG(QWidget *parent);
    virtual ~RecipientLineNG() {}

    void activate() Q_DECL_OVERRIDE;
    bool isActive() const Q_DECL_OVERRIDE;

    bool isEmpty() const Q_DECL_OVERRIDE;
    void clear() Q_DECL_OVERRIDE;

    bool isModified() const Q_DECL_OVERRIDE;
    void clearModified() Q_DECL_OVERRIDE;

    KPIM::MultiplyingLineData::Ptr data() const Q_DECL_OVERRIDE;
    void setData(const KPIM::MultiplyingLineData::Ptr &data) Q_DECL_OVERRIDE;

    void fixTabOrder(QWidget *previous) Q_DECL_OVERRIDE;
    QWidget *tabOut() const Q_DECL_OVERRIDE;

    void moveCompletionPopup() Q_DECL_OVERRIDE;
    void setCompletionMode(KCompletion::CompletionMode mode) Q_DECL_OVERRIDE;

    int setColumnWidth(int w) Q_DECL_OVERRIDE;

    // recipient specific methods
    int recipientsCount() const;

    void setRecipientType(Recipient::Type);
    Recipient::Type recipientType() const;
    QSharedPointer<Recipient> recipient() const;

    /**
     * Sets the config file used for storing recent addresses.
     */
    void setRecentAddressConfig(KConfig *config);

Q_SIGNALS:
    void typeModified(RecipientLineNG *);
    void addRecipient(RecipientLineNG *, const QString &);
    void countChanged();

protected Q_SLOTS:
    void slotEditingFinished();
    void slotTypeModified();
    void analyzeLine(const QString &);

private:
    void dataFromFields();
    void fieldsFromData();
    RecipientComboBox *mCombo;
    RecipientLineEdit *mEdit;
    int mRecipientsCount;
    bool mModified;
    QSharedPointer<Recipient> mData;

private Q_SLOTS:
    void slotAddRecipient(const QString &);
};

}

#endif // RECIPIENTLINE_H
