/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    This file was part of KMail.
    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef MESSAGECOMPOSER_DISTRIBUTIONLISTDIALOG_H
#define MESSAGECOMPOSER_DISTRIBUTIONLISTDIALOG_H

#include <messagecomposer/recipient/recipient.h>
#include <kdialog.h>

class KJob;
class QLineEdit;
class QTreeWidget;

namespace MessageComposer {

class DistributionListDialog : public KDialog
{
    Q_OBJECT
public:
    explicit DistributionListDialog( QWidget *parent );
    ~DistributionListDialog();
    void setRecipients( const Recipient::List & );

public slots:
    void slotUser1();
    void slotTitleChanged( const QString& );

private Q_SLOTS:
    void slotDelayedSetRecipients( KJob* );
    void slotDelayedUser1( KJob* );
    void slotContactGroupCreateJobResult( KJob* );

private:
    void readConfig();
    void writeConfig();

private:
    QLineEdit *mTitleEdit;
    QTreeWidget *mRecipientsList;
};

}

#endif
