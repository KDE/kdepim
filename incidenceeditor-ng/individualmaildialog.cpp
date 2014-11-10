/*
  Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>

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

#include "individualmaildialog.h"

#include <KLocalizedString>

#include <QGridLayout>
#include <QComboBox>
#include <QLabel>

using namespace IncidenceEditorNG;

IndividualMailDialog::IndividualMailDialog(const QString &question, const KCalCore::Attendee::List  &attendees,
        const KGuiItem &buttonYes, const KGuiItem &buttonNo, QWidget *parent)
    : KDialog(parent)
{
    setCaption(i18nc("@title:window", "Group Scheduling Email"));
    setButtons(KDialog::Yes | KDialog::No | KDialog::Details);
    setButtonText(KDialog::Details, i18nc("@action:button show list of attendees", "Individual mailsettings"));
    setButtonGuiItem(KDialog::Yes, buttonYes);
    setButtonGuiItem(KDialog::No, buttonNo);

    QWidget *widget = new QWidget();
    QGridLayout *layout = new QGridLayout(widget);
    int row = 0;
    foreach (const KCalCore::Attendee::Ptr &attendee, attendees) {
        QComboBox *options = new QComboBox();
        options->addItem(i18nc("@item:inlistbox ITIP Messages for one attendee", "Send update"), QVariant(Update));
        options->addItem(i18nc("@item:inlistbox ITIP Messages for one attendee", "Send no update"), QVariant(NoUpdate));
        options->addItem(i18nc("@item:inlistbox ITIP Messages for one attendee", "Edit mail"), QVariant(Edit));
        mAttendeeDecision[attendee] = options;

        layout->addWidget(new QLabel(attendee->fullName()), row, 0);
        layout->addWidget(options, row, 1);
        ++row;
    }
    widget->sizePolicy().setHorizontalStretch(1);
    widget->sizePolicy().setVerticalStretch(1);

    QWidget *mW = new QLabel(question);

    setMainWidget(mW);
    setDetailsWidget(widget);
}

IndividualMailDialog::~IndividualMailDialog()
{

}

KCalCore::Attendee::List IndividualMailDialog::editAttendees() const
{
    KCalCore::Attendee::List edit;
    QList<KCalCore::Attendee::Ptr> attendees = mAttendeeDecision.keys();
    foreach (const KCalCore::Attendee::Ptr &attendee, attendees) {
        int index = mAttendeeDecision[attendee]->currentIndex();
        if (mAttendeeDecision[attendee]->itemData(index, Qt::UserRole) == Edit) {
            edit.append(attendee);
        }
    }
    return edit;
}

KCalCore::Attendee::List IndividualMailDialog::updateAttendees() const
{
    KCalCore::Attendee::List update;
    QList<KCalCore::Attendee::Ptr> attendees = mAttendeeDecision.keys();
    foreach (const KCalCore::Attendee::Ptr &attendee, attendees) {
        int index = mAttendeeDecision[attendee]->currentIndex();
        if (mAttendeeDecision[attendee]->itemData(index, Qt::UserRole) == Update) {
            update.append(attendee);
        }
    }
    return update;
}
