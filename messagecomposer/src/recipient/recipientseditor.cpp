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

#include "recipientseditor.h"

#include "recipient.h"
#include "recipientline.h"
#include "recipientseditorsidewidget.h"

#include "settings/messagecomposersettings.h"
#include <MessageComposer/DistributionListDialog>

#include "messagecomposer_debug.h"

#include <KMime/Headers>
#include <KLocalizedString>
#include <KMessageBox>

#include <QLayout>
#include <QDebug>

using namespace MessageComposer;
using namespace KPIM;

RecipientLineFactory::RecipientLineFactory(QObject *parent)
    : KPIM::MultiplyingLineFactory(parent)
{

}

KPIM::MultiplyingLine *RecipientLineFactory::newLine(QWidget *p)
{
    RecipientLineNG *line = new RecipientLineNG(p);
    if (qobject_cast<RecipientsEditor *>(parent())) {
        connect(line, SIGNAL(addRecipient(RecipientLineNG*,QString)), qobject_cast<RecipientsEditor *>(parent()), SLOT(addRecipient(RecipientLineNG*,QString)));
    } else {
        qCWarning(MESSAGECOMPOSER_LOG) << "RecipientLineFactory::newLine: We can't connect to new line" << parent();
    }
    return line;
}

int RecipientLineFactory::maximumRecipients()
{
    return MessageComposer::MessageComposerSettings::self()->maximumRecipients();
}

class MessageComposer::RecipientsEditorPrivate
{
public:
    RecipientsEditorPrivate()
        : mRecentAddressConfig(Q_NULLPTR),
          mSideWidget(Q_NULLPTR)
    {

    }
    KConfig *mRecentAddressConfig;
    RecipientsEditorSideWidget *mSideWidget;
};

RecipientsEditor::RecipientsEditor(QWidget *parent)
    : MultiplyingLineEditor(new RecipientLineFactory(Q_NULLPTR), parent),
      d(new MessageComposer::RecipientsEditorPrivate)
{
    factory()->setParent(this);   // HACK: can't use 'this' above since it's not yet constructed at that point
    d->mSideWidget = new RecipientsEditorSideWidget(this, this);

    layout()->addWidget(d->mSideWidget);

    connect(d->mSideWidget, &RecipientsEditorSideWidget::pickedRecipient, this, &RecipientsEditor::slotPickedRecipient);
    connect(d->mSideWidget, &RecipientsEditorSideWidget::saveDistributionList, this, &RecipientsEditor::saveDistributionList);

    //   connect( mView, SIGNAL(focusRight()),
    //     d->mSideWidget, SLOT(setFocus()) );

    connect(this, &RecipientsEditor::lineAdded, this, &RecipientsEditor::slotLineAdded);
    connect(this, &RecipientsEditor::lineDeleted, this, &RecipientsEditor::slotLineDeleted);

    addData(); // one defaut line
}

RecipientsEditor::~RecipientsEditor()
{
    delete d;
}

bool RecipientsEditor::addRecipient(const QString &recipient, Recipient::Type type)
{
    return addData(Recipient::Ptr(new Recipient(recipient, type)));
}

void RecipientsEditor::addRecipient(RecipientLineNG *line, const QString &recipient)
{
    addRecipient(recipient, line->recipientType());
}

void RecipientsEditor::setRecipientString(const QVector< KMime::Types::Mailbox > &mailboxes, Recipient::Type type)
{
    int count = 1;

    foreach (const KMime::Types::Mailbox &mailbox, mailboxes) {
        if (count++ > MessageComposer::MessageComposerSettings::self()->maximumRecipients()) {
            KMessageBox::sorry(this,
                               i18ncp("@info:status",
                                      "Truncating recipients list to %2 of %1 entry.",
                                      "Truncating recipients list to %2 of %1 entries.",
                                      mailboxes.count(),
                                      MessageComposer::MessageComposerSettings::self()->maximumRecipients()));
            break;
        }
        addRecipient(mailbox.prettyAddress(KMime::Types::Mailbox::QuoteWhenNecessary), type);
    }
}

Recipient::List RecipientsEditor::recipients() const
{
    QList<MultiplyingLineData::Ptr> dataList = allData();
    Recipient::List recList;
    foreach (const MultiplyingLineData::Ptr &datum, dataList) {
        Recipient::Ptr rec = qSharedPointerDynamicCast<Recipient>(datum);
        if (!rec) {
            continue;
        }
        recList << rec;
    }
    return recList;
}

Recipient::Ptr RecipientsEditor::activeRecipient() const
{
    return qSharedPointerDynamicCast<Recipient>(activeData());
}

QString RecipientsEditor::recipientString(Recipient::Type type) const
{
    return recipientStringList(type).join(QStringLiteral(", "));
}

QStringList RecipientsEditor::recipientStringList(Recipient::Type type) const
{
    QStringList selectedRecipients;
    foreach (const Recipient::Ptr &r, recipients()) {
        if (r->type() == type) {
            selectedRecipients << r->email();
        }
    }
    return selectedRecipients;
}

void RecipientsEditor::removeRecipient(const QString &recipient, Recipient::Type type)
{
    // search a line which matches recipient and type
    QListIterator<MultiplyingLine *> it(lines());
    MultiplyingLine *line = Q_NULLPTR;
    while (it.hasNext()) {
        line = it.next();
        RecipientLineNG *rec = qobject_cast< RecipientLineNG * >(line);
        if (rec) {
            if ((rec->recipient()->email() == recipient) &&
                    (rec->recipientType() == type)) {
                break;
            }
        }
    }
    if (line) {
        line->slotPropagateDeletion();
    }
}

void RecipientsEditor::saveDistributionList()
{
    QScopedPointer<MessageComposer::DistributionListDialog> dlg(new MessageComposer::DistributionListDialog(this));
    dlg->setRecipients(recipients());
    dlg->exec();
}

void RecipientsEditor::selectRecipients()
{
    d->mSideWidget->pickRecipient();
}

void MessageComposer::RecipientsEditor::setRecentAddressConfig(KConfig *config)
{
    d->mRecentAddressConfig = config;
    if (config) {
        MultiplyingLine *line;
        foreach (line, lines()) {
            RecipientLineNG *rec = qobject_cast< RecipientLineNG * >(line);
            if (rec) {
                rec->setRecentAddressConfig(config);
            }
        }
    }
}

void MessageComposer::RecipientsEditor::slotPickedRecipient(const Recipient &rec, bool &tooManyAddress)
{
    Recipient::Type t = rec.type();
    tooManyAddress = addRecipient(rec.email(), t == Recipient::Undefined ? Recipient::To : t);
    mModified = true;
}

RecipientsPicker *RecipientsEditor::picker() const
{
    return d->mSideWidget->picker();
}

void RecipientsEditor::slotLineAdded(MultiplyingLine *line)
{
    // subtract 1 here, because we want the number of lines
    // before this line was added.
    int count = lines().size() - 1;
    RecipientLineNG *rec = qobject_cast<RecipientLineNG *>(line);
    if (!rec) {
        return;
    }

    if (d->mRecentAddressConfig) {
        rec->setRecentAddressConfig(d->mRecentAddressConfig);
    }

    if (count > 0) {
        if (count == 1) {
            RecipientLineNG *last_rec = qobject_cast< RecipientLineNG * >(lines().last());
            if (last_rec && last_rec->recipientType() == Recipient::Bcc) {
                rec->setRecipientType(Recipient::To);
            } else {
                rec->setRecipientType(Recipient::Cc);
            }
        } else {
            RecipientLineNG *last_rec = qobject_cast< RecipientLineNG * >(lines().at(lines().count() - 2));
            if (last_rec) {
                rec->setRecipientType(last_rec->recipientType());
            }
        }
        line->fixTabOrder(lines().last()->tabOut());
    }
    connect(rec, &RecipientLineNG::countChanged, this, &RecipientsEditor::slotCalculateTotal);
    //   connect( rec->mEdit, SIGNAL(textChanged(QString)),
    //            SLOT(slotCalculateTotal()) );
}

void RecipientsEditor::slotLineDeleted(int pos)
{
    bool atLeastOneToLine = false;
    int firstCC = -1;
    for (int i = pos; i < lines().count(); ++i) {
        MultiplyingLine *line = lines().at(i);
        RecipientLineNG *rec = qobject_cast< RecipientLineNG * >(line);
        if (rec) {
            if (rec->recipientType() == Recipient::To) {
                atLeastOneToLine = true;
            } else if ((rec->recipientType() == Recipient::Cc) && (firstCC < 0)) {
                firstCC = i;
            }
        }
    }

    if (!atLeastOneToLine && (firstCC >= 0)) {
        RecipientLineNG *firstCCLine = qobject_cast< RecipientLineNG * >(lines().at(firstCC));
        if (firstCCLine) {
            firstCCLine->setRecipientType(Recipient::To);
        }
    }

    slotCalculateTotal();
}

void RecipientsEditor::slotCalculateTotal()
{
    int count = 0;
    int empty = 0;

    MultiplyingLine *line;
    foreach (line, lines()) {
        RecipientLineNG *rec = qobject_cast< RecipientLineNG * >(line);
        if (rec) {
            if (rec->isEmpty()) {
                ++empty;
            } else {
                count += rec->recipientsCount();
            }
        }
    }

    // We always want at least one empty line
    if (empty == 0) {
        addData();
    }

    // update the side widget
    d->mSideWidget->setTotal(count, lines().count());
}

RecipientLineNG *RecipientsEditor::activeLine() const
{
    MultiplyingLine *line = MultiplyingLineEditor::activeLine();
    return qobject_cast< RecipientLineNG * >(line);
}

