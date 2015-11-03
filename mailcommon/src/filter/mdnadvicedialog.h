/*
  Copyright (c) 2009 Michael Leupold <lemma@confuego.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef MAILCOMMON_MDNADVICEDIALOG_H
#define MAILCOMMON_MDNADVICEDIALOG_H

#include "mailcommon_export.h"

#include <MessageComposer/MessageFactory>
#include "mailcommon/mdnstateattribute.h"

#include <KMime/KMimeMessage>

#include <QDialog>

namespace MailCommon
{

class MDNAdviceHelper : public QObject
{
    Q_OBJECT
public:
    static MDNAdviceHelper *instance()
    {
        if (!s_instance) {
            s_instance = new MDNAdviceHelper;
        }

        return s_instance;
    }

    /**
     * Checks the MDN headers to see if the user needs to be asked for any
     * confirmations. Will ask the user if action is required.
     *
     * Returns whether to send an MDN or not, and the sending mode for the MDN
     * to be created.
     *
     * Will also set the MailCommon::MDNStateAttribute on the given item
     * to what the user has selected.
     */
    QPair<bool, KMime::MDN::SendingMode>checkAndSetMDNInfo(
        const Akonadi::Item &item, KMime::MDN::DispositionType d, bool forceSend = false);

    MailCommon::MDNStateAttribute::MDNSentState dispositionToSentState(
        KMime::MDN::DispositionType d);

private:
    explicit MDNAdviceHelper(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {
    }

    virtual ~MDNAdviceHelper()
    {
    }

    int requestAdviceOnMDN(const char *what);
    MessageComposer::MDNAdvice questionIgnoreSend(const QString &text, bool canDeny);

    static MDNAdviceHelper *s_instance;
};

class MAILCOMMON_EXPORT MDNAdviceDialog : public QDialog
{
    Q_OBJECT

public:
    MDNAdviceDialog(const QString &text, bool canDeny, QWidget *parent = Q_NULLPTR);
    ~MDNAdviceDialog();

    MessageComposer::MDNAdvice result() const;

private Q_SLOTS:
    void slotUser1Clicked();
    void slotUser2Clicked();
    void slotYesClicked();

private:
    MessageComposer::MDNAdvice m_result;

protected:

    // Reimplemented
    void slotButtonClicked(int button);
};

}

#endif
