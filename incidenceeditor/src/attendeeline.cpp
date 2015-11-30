/*
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
  Copyright (c) 2009-2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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

#include "attendeeline.h"
#include "attendeedata.h"

#include <KCalUtils/Stringify>

#include <KEmailAddress>

#include <KCompletionBox>
#include "incidenceeditor_debug.h"
#include <KLocalizedString>
#include <KIconLoader>

#include <QBoxLayout>
#include <QKeyEvent>
#include <QMenu>

using namespace IncidenceEditorNG;

typedef QPair<QString, QIcon> TextIconPair;

AttendeeComboBox::AttendeeComboBox(QWidget *parent)
    : QToolButton(parent), mMenu(new QMenu(this)), mCurrentIndex(-1)
{
    setPopupMode(QToolButton::InstantPopup);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setMenu(mMenu);
}

void AttendeeComboBox::addItem(const QIcon &icon, const QString &text)
{
    mList.append(TextIconPair(text, icon));
    if (mCurrentIndex == -1) {
        setCurrentIndex(0);
    }
    int index = mList.size() - 1;
    QAction *act = menu()->addAction(icon, text, this, SLOT(slotActionTriggered()));
    act->setData(index);
}

void AttendeeComboBox::addItems(const QStringList &texts)
{
    foreach (const QString &str, texts) {
        addItem(QIcon(), str);
    }
    if (mCurrentIndex == -1) {
        setCurrentIndex(0);
    }
}

int AttendeeComboBox::currentIndex() const
{
    return mCurrentIndex;
}

void AttendeeComboBox::clear()
{
    mCurrentIndex = -1;
    mMenu->clear();
    mList.clear();
}

void AttendeeComboBox::setCurrentIndex(int index)
{
    Q_ASSERT(index < mList.size());
    const int old = mCurrentIndex;
    mCurrentIndex = index;
    setIcon(mList.at(index).second);
    setToolTip(mList.at(index).first);
    if (old != index) {
        Q_EMIT itemChanged();
    }
}

void AttendeeComboBox::slotActionTriggered()
{
    int index = qobject_cast<QAction *> (sender())->data().toInt();
    setCurrentIndex(index);
}

void AttendeeComboBox::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Left) {
        Q_EMIT leftPressed();
    } else if (ev->key() == Qt::Key_Right) {
        Q_EMIT rightPressed();
    } else if (!mMenu->isVisible() && (
                   ev->key() == Qt::Key_Down ||
                   ev->key() == Qt::Key_Space))  {
        showMenu();
    } else {
        QToolButton::keyPressEvent(ev);
    }
}

AttendeeLineEdit::AttendeeLineEdit(QWidget *parent)
    : AddresseeLineEdit(parent, true)
{
}

void AttendeeLineEdit::keyPressEvent(QKeyEvent *ev)
{
    if ((ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return) &&
            !completionBox()->isVisible()) {
        Q_EMIT downPressed();
        KPIM::AddresseeLineEdit::keyPressEvent(ev);
    } else if (ev->key() == Qt::Key_Backspace  &&  text().isEmpty()) {
        ev->accept();
        Q_EMIT deleteMe();
    } else if (ev->key() == Qt::Key_Left && cursorPosition() == 0 &&
               !ev->modifiers().testFlag(Qt::ShiftModifier)) {
        // Shift would be pressed during selection
        Q_EMIT leftPressed();
    } else if (ev->key() == Qt::Key_Right && cursorPosition() == (int)text().length() &&
               !ev->modifiers().testFlag(Qt::ShiftModifier)) {
        // Shift would be pressed during selection
        Q_EMIT rightPressed();
    } else if (ev->key() == Qt::Key_Down) {
        Q_EMIT downPressed();
    } else if (ev->key() == Qt::Key_Up) {
        Q_EMIT upPressed();
    } else {
        KPIM::AddresseeLineEdit::keyPressEvent(ev);
    }
}

AttendeeLine::AttendeeLine(QWidget *parent)
    : MultiplyingLine(parent),
      mRoleCombo(new AttendeeComboBox(this)),
      mStateCombo(new AttendeeComboBox(this)),
      mResponseCombo(new AttendeeComboBox(this)),
      mEdit(new AttendeeLineEdit(this)),
      mData(new AttendeeData(QString(), QString())),
      mModified(false)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QBoxLayout *topLayout = new QHBoxLayout(this);
    topLayout->setMargin(0);
    mRoleCombo->addItem(QIcon::fromTheme(QStringLiteral("meeting-participant")),
                        KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::ReqParticipant));
    mRoleCombo->addItem(QIcon::fromTheme(QStringLiteral("meeting-participant-optional")),
                        KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::OptParticipant));
    mRoleCombo->addItem(QIcon::fromTheme(QStringLiteral("meeting-observer")),
                        KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::NonParticipant));
    mRoleCombo->addItem(QIcon::fromTheme(QStringLiteral("meeting-chair")),
                        KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::Chair));

    mResponseCombo->addItem(QIcon::fromTheme(QStringLiteral("meeting-participant-request-response")),
                            i18nc("@item:inlistbox", "Request Response"));
    mResponseCombo->addItem(QIcon::fromTheme(QStringLiteral("meeting-participant-no-response")),
                            i18nc("@item:inlistbox", "Request No Response"));

    mEdit->setToolTip(i18nc("@info:tooltip",
                            "Enter the name or email address of the attendee."));
    mEdit->setClearButtonShown(true);

    mStateCombo->setWhatsThis(i18nc("@info:whatsthis",
                                    "Edits the current attendance status of the attendee."));

    mRoleCombo->setWhatsThis(i18nc("@info:whatsthis",
                                   "Edits the role of the attendee."));

    mEdit->setWhatsThis(i18nc("@info:whatsthis",
                              "The email address or name of the attendee. An invitation "
                              "can be sent to the user if an email address is provided."));

    setActions(EventActions);

    mResponseCombo->setToolTip(i18nc("@info:tooltip", "Request a response from the attendee"));
    mResponseCombo->setWhatsThis(i18nc("@info:whatsthis",
                                       "Edits whether to send an email to the "
                                       "attendee to request a response concerning "
                                       "attendance."));

    // add them to the layout in the correct order
    topLayout->addWidget(mRoleCombo);
    topLayout->addWidget(mEdit);
    topLayout->addWidget(mStateCombo);
    topLayout->addWidget(mResponseCombo);

    connect(mEdit, &AttendeeLineEdit::returnPressed, this, &AttendeeLine::slotReturnPressed);
    connect(mEdit, &AttendeeLineEdit::deleteMe, this, &AttendeeLine::slotPropagateDeletion, Qt::QueuedConnection);
    connect(mEdit, &AttendeeLineEdit::textChanged, this, &AttendeeLine::slotTextChanged, Qt::QueuedConnection);
    connect(mEdit, &AttendeeLineEdit::upPressed, this, &AttendeeLine::slotFocusUp);
    connect(mEdit, &AttendeeLineEdit::downPressed, this, &AttendeeLine::slotFocusDown);

    connect(mRoleCombo, &AttendeeComboBox::rightPressed, mEdit, static_cast<void (AttendeeLineEdit::*)()>(&AttendeeLineEdit::setFocus));
    connect(mEdit, &AttendeeLineEdit::leftPressed, mRoleCombo, static_cast<void (AttendeeComboBox::*)()>(&AttendeeComboBox::setFocus));

    connect(mEdit, &AttendeeLineEdit::rightPressed, mStateCombo, static_cast<void (AttendeeComboBox::*)()>(&AttendeeComboBox::setFocus));
    connect(mStateCombo, &AttendeeComboBox::leftPressed, mEdit,  static_cast<void (AttendeeLineEdit::*)()>(&AttendeeLineEdit::setFocus));

    connect(mStateCombo, &AttendeeComboBox::rightPressed, mResponseCombo, static_cast<void (AttendeeComboBox::*)()>(&AttendeeComboBox::setFocus));

    connect(mResponseCombo, &AttendeeComboBox::leftPressed, mStateCombo, static_cast<void (AttendeeComboBox::*)()>(&AttendeeComboBox::setFocus));
    connect(mResponseCombo, &AttendeeComboBox::rightPressed, this, &AttendeeLine::rightPressed);

    connect(mEdit, &AttendeeLineEdit::editingFinished, this, &AttendeeLine::slotHandleChange, Qt::QueuedConnection);
    connect(mEdit, &AttendeeLineEdit::textCompleted, this, &AttendeeLine::slotHandleChange, Qt::QueuedConnection);
    connect(mEdit, &AttendeeLineEdit::clearButtonClicked, this, &AttendeeLine::slotPropagateDeletion, Qt::QueuedConnection);

    connect(mRoleCombo, &AttendeeComboBox::itemChanged, this, &AttendeeLine::slotComboChanged);
    connect(mStateCombo, &AttendeeComboBox::itemChanged, this, &AttendeeLine::slotComboChanged);
    connect(mResponseCombo, &AttendeeComboBox::itemChanged, this, &AttendeeLine::slotComboChanged);

}

void AttendeeLine::activate()
{
    mEdit->setFocus();
}

void AttendeeLine::clear()
{
    mEdit->clear();
    mRoleCombo->setCurrentIndex(0);
    mStateCombo->setCurrentIndex(0);
    mResponseCombo->setCurrentIndex(0);
    mUid.clear();
}

void AttendeeLine::clearModified()
{
    mModified = false;
    mEdit->setModified(false);
}

KPIM::MultiplyingLineData::Ptr AttendeeLine::data() const
{
    if (isModified()) {
        const_cast<AttendeeLine *>(this)->dataFromFields();
    }
    return mData;
}

void AttendeeLine::dataFromFields()
{
    if (!mData) {
        return;
    }

    KCalCore::Attendee::Ptr oldAttendee(mData->attendee());

    QString email, name;
    KEmailAddress::extractEmailAddressAndName(mEdit->text(), email, name);

    mData->setName(name);
    mData->setEmail(email);

    mData->setRole(AttendeeData::Role(mRoleCombo->currentIndex()));
    mData->setStatus(AttendeeData::PartStat(mStateCombo->currentIndex()));
    mData->setRSVP(mResponseCombo->currentIndex() == 0);
    mData->setUid(mUid);

    clearModified();
    if (!(oldAttendee == mData->attendee()) && !email.isEmpty()) {
        // if email is empty, we don't want to update anything
        qCDebug(INCIDENCEEDITOR_LOG) << oldAttendee->email() << mData->email();
        Q_EMIT changed(oldAttendee, mData->attendee());
    }
}

void AttendeeLine::fieldsFromData()
{
    if (!mData) {
        return;
    }

    mEdit->setText(mData->fullName());
    mRoleCombo->setCurrentIndex(mData->role());
    AttendeeData::PartStat partStat = mData->status();
    if (partStat != AttendeeData::None) {
        mStateCombo->setCurrentIndex(partStat);
    } else {
        mStateCombo->setCurrentIndex(AttendeeData::NeedsAction);
    }
    mResponseCombo->setCurrentIndex(mData->RSVP() ? 0 : 1);
    mUid = mData->uid();
}

void AttendeeLine::fixTabOrder(QWidget *previous)
{
    setTabOrder(previous, mRoleCombo);
    setTabOrder(mRoleCombo, mEdit);
    setTabOrder(mEdit, mStateCombo);
    setTabOrder(mStateCombo, mResponseCombo);
}

QWidget *AttendeeLine::tabOut() const
{
    return mResponseCombo;
}

bool AttendeeLine::isActive() const
{
    return mEdit->hasFocus();
}

bool AttendeeLine::isEmpty() const
{
    return mEdit->text().isEmpty();
}

bool AttendeeLine::isModified() const
{
    return mModified || mEdit->isModified();
}

void AttendeeLine::moveCompletionPopup()
{
    if (mEdit->completionBox(false)) {
        if (mEdit->completionBox()->isVisible()) {
            // ### trigger moving, is there a nicer way to do that?
            mEdit->completionBox()->hide();
            mEdit->completionBox()->show();
        }
    }
}

int AttendeeLine::setColumnWidth(int w)
{
    w = qMax(w, mRoleCombo->sizeHint().width());
    mRoleCombo->setFixedWidth(w);
    mRoleCombo->updateGeometry();
    parentWidget()->updateGeometry();
    return w;
}

void AttendeeLine::setActions(AttendeeActions actions)
{
    mStateCombo->clear();
    return;
    if (actions == EventActions) {
        mStateCombo->addItem(QIcon::fromTheme(QStringLiteral("task-attention")),
                             KCalUtils::Stringify::attendeeStatus(AttendeeData::NeedsAction));
        mStateCombo->addItem(QIcon::fromTheme(QStringLiteral("task-accepted")),
                             KCalUtils::Stringify::attendeeStatus(AttendeeData::Accepted));
        mStateCombo->addItem(QIcon::fromTheme(QStringLiteral("task-reject")),
                             KCalUtils::Stringify::attendeeStatus(AttendeeData::Declined));
        mStateCombo->addItem(QIcon::fromTheme(QStringLiteral("task-attempt")),
                             KCalUtils::Stringify::attendeeStatus(AttendeeData::Tentative));
        mStateCombo->addItem(QIcon::fromTheme(QStringLiteral("task-delegate")),
                             KCalUtils::Stringify::attendeeStatus(AttendeeData::Delegated));
    } else {
        mStateCombo->addItem(QIcon::fromTheme(QStringLiteral("task-attention")),
                             KCalUtils::Stringify::attendeeStatus(AttendeeData::NeedsAction));
        mStateCombo->addItem(QIcon::fromTheme(QStringLiteral("task-accepted")),
                             KCalUtils::Stringify::attendeeStatus(AttendeeData::Accepted));
        mStateCombo->addItem(QIcon::fromTheme(QStringLiteral("task-reject")),
                             KCalUtils::Stringify::attendeeStatus(AttendeeData::Declined));
        mStateCombo->addItem(QIcon::fromTheme(QStringLiteral("task-attempt")),
                             KCalUtils::Stringify::attendeeStatus(AttendeeData::Tentative));
        mStateCombo->addItem(QIcon::fromTheme(QStringLiteral("task-delegate")),
                             KCalUtils::Stringify::attendeeStatus(AttendeeData::Delegated));
        mStateCombo->addItem(QIcon::fromTheme(QStringLiteral("task-complete")),
                             KCalUtils::Stringify::attendeeStatus(AttendeeData::Completed));
        mStateCombo->addItem(QIcon::fromTheme(QStringLiteral("task-ongoing")),
                             KCalUtils::Stringify::attendeeStatus(AttendeeData::InProcess));
    }
}

void AttendeeLine::setCompletionMode(KCompletion::CompletionMode mode)
{
    mEdit->setCompletionMode(mode);
}

void AttendeeLine::setData(const KPIM::MultiplyingLineData::Ptr &data)
{
    AttendeeData::Ptr attendee = qSharedPointerDynamicCast<AttendeeData>(data);
    if (!attendee) {
        return;
    }
    mData = attendee;
    fieldsFromData();
}

void AttendeeLine::slotHandleChange()
{
    if (mEdit->text().isEmpty()) {
        Q_EMIT deleteLine(this);
    } else {
        // has bad side-effects, and I have no idea what this was supposed to be doing
//    mEdit->setCursorPosition( 0 );
        Q_EMIT editingFinished(this);
        dataFromFields();
    }
}

void AttendeeLine::slotTextChanged(const QString &str)
{
    Q_UNUSED(str);
    mModified = true;
    Q_EMIT changed(); // TODO: This doesn't seem connected to anywhere in incidenceattendee.cpp.
    // but the important code is run in slotHandleChange() anyway so we don't see any bug
}

void AttendeeLine::slotComboChanged()
{
    mModified = true;
    // If mUid is empty, we're still populating the widget, don't write fields to data yet
    if (!mUid.isEmpty()) {
        dataFromFields();
    }
}

void AttendeeLine::aboutToBeDeleted()
{
    if (!mData) {
        return;
    }

    Q_EMIT changed(mData->attendee(), KCalCore::Attendee::Ptr(new KCalCore::Attendee(QLatin1String(""), QLatin1String(""))));
}

