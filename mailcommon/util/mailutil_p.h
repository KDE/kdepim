/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net,
    Author Tobias Koenig <tokoe@kdab.com>

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

#ifndef MAILCOMMON_MAILUTIL_P_H
#define MAILCOMMON_MAILUTIL_P_H

#include <QDialog>
#include <KLocalizedString>
#include <KSeparator>

#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

namespace MailCommon
{

class AttachmentSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    enum Type {
        AttachAsLink,
        AttachInline,
        AttachWithoutAttachments
    };

    explicit AttachmentSelectionDialog(QWidget *parent = 0)
        : QDialog(parent),
          mType(AttachAsLink)
    {
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
        QWidget *mainWidget = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout;
        setLayout(mainLayout);
        mainLayout->addWidget(mainWidget);
        QPushButton *user1Button = new QPushButton;
        buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
        QPushButton *user2Button = new QPushButton;
        buttonBox->addButton(user2Button, QDialogButtonBox::ActionRole);
        QPushButton *user3Button = new QPushButton;
        buttonBox->addButton(user3Button, QDialogButtonBox::ActionRole);
        connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
        connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
        connect(user1Button, SIGNAL(clicked()), this, SLOT(slotUser1Clicked()));
        connect(user2Button, SIGNAL(clicked()), this, SLOT(slotUser2Clicked()));
        connect(user3Button, SIGNAL(clicked()), this, SLOT(slotUser3Clicked()));

        setWindowTitle(i18n("Create Todo/Reminder"));
        user1Button->setText(i18n("Attach inline without attachments"));
        user2Button->setText(i18n("Attach &inline"));
        user3Button->setText(i18n("Attach as &link"));
        QVBoxLayout *lay = new QVBoxLayout;
        QWidget *w = new QWidget;
        w->setLayout(lay);
        lay->addWidget(new QLabel(i18n("How should the email be attached?")));
        lay->addWidget(new KSeparator);
        mainLayout->addWidget(w);
        mainLayout->addWidget(buttonBox);
    }

    Type attachmentType() const
    {
        return mType;
    }

protected Q_SLOTS:
    void slotUser1Clicked()
    {
        mType = AttachWithoutAttachments;
        accept();
    }
    void slotUser2Clicked()
    {
        mType = AttachInline;
        accept();
    }
    void slotUser3Clicked()
    {
        mType = AttachAsLink;
        accept();
    }

private:
    Type mType;
};

}

#endif
