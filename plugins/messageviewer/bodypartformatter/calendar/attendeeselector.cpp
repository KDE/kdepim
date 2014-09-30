/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#include "attendeeselector.h"

#include <libkdepim/addressline/addresseelineedit.h>
#include <KPIMUtils/Email>

#include <KLocalizedString>
#include <QPushButton>
#include <kguiitem.h>
#include <KGuiItem>
#include <KStandardGuiItem>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QVBoxLayout>

AttendeeSelector::AttendeeSelector(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Select Attendees"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AttendeeSelector::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AttendeeSelector::reject);
    mainLayout->addWidget(buttonBox);

    ui.setupUi(mainWidget);

    KGuiItem::assign(ui.addButton, KStandardGuiItem::add());
    connect(ui.addButton, &QPushButton::clicked, this, &AttendeeSelector::addClicked);
    KGuiItem::assign(ui.removeButton, KStandardGuiItem::remove());
    connect(ui.removeButton, &QPushButton::clicked, this, &AttendeeSelector::removeClicked);

    ui.attendeeEdit->setPlaceholderText(i18n("Click to add a new attendee"));
    connect(ui.attendeeEdit, &KPIM::AddresseeLineEdit::textChanged, this, &AttendeeSelector::textChanged);
    connect(ui.attendeeEdit, &KPIM::AddresseeLineEdit::returnPressed, this, &AttendeeSelector::addClicked);

    connect(ui.attendeeList, &QListWidget::itemSelectionChanged, this, &AttendeeSelector::selectionChanged);
    mOkButton->setEnabled(false);
}

QStringList AttendeeSelector::attendees() const
{
    QStringList rv;
    const int numberOfAttendee(ui.attendeeList->count());
    for (int i = 0; i < numberOfAttendee; ++i) {
        const QString addr = ui.attendeeList->item(i)->text();

        // Build a nice address for this attendee including the CN.
        QString tname, temail;
        KPIMUtils::extractEmailAddressAndName(addr, temail, tname);    // ignore return value
        // which is always false
        rv << temail;
    }
    return rv;
}

void AttendeeSelector::addClicked()
{
    if (!ui.attendeeEdit->text().isEmpty()) {
        ui.attendeeList->addItem(ui.attendeeEdit->text());
    }
    ui.attendeeEdit->clear();
    mOkButton->setEnabled(true);
}

void AttendeeSelector::removeClicked()
{
    delete ui.attendeeList->takeItem(ui.attendeeList->currentRow());
    mOkButton->setEnabled((ui.attendeeList->count() > 0));
}

void AttendeeSelector::textChanged(const QString &text)
{
    ui.addButton->setEnabled(!text.isEmpty());
}

void AttendeeSelector::selectionChanged()
{
    ui.removeButton->setEnabled(ui.attendeeList->currentItem());
}

