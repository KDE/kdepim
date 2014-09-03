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

#include <KDialog>
#include <KLocalizedString>
#include <KSeparator>

#include <QLabel>
#include <QVBoxLayout>

namespace MailCommon
{

class AttachmentSelectionDialog : public KDialog
{
    Q_OBJECT

public:
    enum Type {
        AttachAsLink,
        AttachInline,
        AttachWithoutAttachments
    };

    explicit AttachmentSelectionDialog(QWidget *parent = 0)
        : KDialog(parent),
          mButtonCode(KDialog::Cancel)
    {
        setButtons(User1 | User2 | User3 | Cancel);
        setWindowTitle(i18n("Create Todo/Reminder"));
        setButtonText(User1, i18n("Attach inline without attachments"));
        setButtonText(User2, i18n("Attach &inline"));
        setButtonText(User3, i18n("Attach as &link"));
        QVBoxLayout *lay = new QVBoxLayout;
        QWidget *w = new QWidget;
        w->setLayout(lay);
        lay->addWidget(new QLabel(i18n("How should the email be attached?")));
        lay->addWidget(new KSeparator);
        setMainWidget(w);
    }

    Type attachmentType() const
    {
        switch (mButtonCode) {
        case User1:
            return AttachWithoutAttachments;
        case User2:
            return AttachInline;
        case User3:
        default:
            return AttachAsLink;
        }
    }

protected Q_SLOTS:
    void slotButtonClicked(int button)
    {
        mButtonCode = static_cast<KDialog::ButtonCode>(button);

        if (mButtonCode == User1 || mButtonCode == User2 || mButtonCode == User3) {
            accept();
        }

        KDialog::slotButtonClicked(button);
    }

private:
    KDialog::ButtonCode mButtonCode;
};

}

#endif
