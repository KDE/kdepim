/*
 * Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * As a special exception, permission is given to link this program
 * with any edition of Qt, and distribute the resulting executable,
 * without including the source code for Qt in the source distribution.
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
    QSizePolicy sizePolicy = widget->sizePolicy();
    sizePolicy.setHorizontalStretch(1);
    sizePolicy.setVerticalStretch(1);
    widget->setSizePolicy(sizePolicy);

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
    for (auto it = mAttendeeDecision.cbegin(), end = mAttendeeDecision.cend(); it != end; ++it) {
        int index = it.value()->currentIndex();
        if (it.value()->itemData(index, Qt::UserRole) == Edit) {
            edit.append(it.key());
        }
    }
    return edit;
}

KCalCore::Attendee::List IndividualMailDialog::updateAttendees() const
{
    KCalCore::Attendee::List update;
    for (auto it = mAttendeeDecision.cbegin(), end = mAttendeeDecision.cend(); it != end; ++it) {
        int index = it.value()->currentIndex();
        if (it.value()->itemData(index, Qt::UserRole) == Update) {
            update.append(it.key());
        }
    }
    return update;
}
