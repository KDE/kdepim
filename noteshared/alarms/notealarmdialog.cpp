/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 2005, Michael Brade <brade@kde.org>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

 In addition, as a special exception, the copyright holders give
 permission to link the code of this program with any edition of
 the Qt library by Trolltech AS, Norway (or with modified versions
 of Qt that use the same license as Qt), and distribute linked
 combinations including the two.  You must obey the GNU General
 Public License in all respects for all of the code used other than
 Qt.  If you modify this file, you may extend this exception to
 your version of the file, but you are not obligated to do so.  If
 you do not wish to do so, delete this exception statement from
 your version.
*******************************************************************/

#include "notealarmdialog.h"

#include <KDateComboBox>
#include <KLocalizedString>
#include <KTimeComboBox>
#include <QVBoxLayout>
#include <KDateTime>

#include <QDateTime>
#include <QButtonGroup>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace NoteShared;
NoteAlarmDialog::NoteAlarmDialog(const QString &caption, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(caption);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &NoteAlarmDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &NoteAlarmDialog::reject);
    mainLayout->addWidget(buttonBox);
    QWidget *page = new QWidget(this);
    QVBoxLayout *pageVBoxLayout = new QVBoxLayout(page);
    pageVBoxLayout->setMargin(0);

    m_buttons = new QButtonGroup(this);
    QGroupBox *group = new QGroupBox(i18n("Scheduled Alarm"), page);
    pageVBoxLayout->addWidget(group);
    QVBoxLayout *layout = new QVBoxLayout;
    QRadioButton *none = new QRadioButton(i18n("&No alarm"));
    layout->addWidget(none);
    m_buttons->addButton(none, 0);

    group->setLayout(layout);

    QWidget *at = new QWidget;
    QHBoxLayout *atHBoxLayout = new QHBoxLayout(at);
    atHBoxLayout->setMargin(0);
    QRadioButton *label_at = new QRadioButton(i18n("Alarm &at:"), at);
    atHBoxLayout->addWidget(label_at);
    m_atDate = new KDateComboBox(at);
    atHBoxLayout->addWidget(m_atDate);
    m_atTime = new KTimeComboBox(at);
    atHBoxLayout->addWidget(m_atTime);
    const QDateTime dateTime = QDateTime::currentDateTime();
    m_atDate->setMinimumDate(dateTime.date());
    m_atTime->setMinimumTime(dateTime.time());
    atHBoxLayout->setStretchFactor(m_atDate, 1);
    layout->addWidget(at);
    m_buttons->addButton(label_at, 1);

    connect(m_buttons, SIGNAL(buttonClicked(int)),
            SLOT(slotButtonChanged(int)));
    connect(okButton, &QPushButton::clicked, this, &NoteAlarmDialog::accept);
    m_buttons->button(0)->setChecked(true);
    slotButtonChanged(m_buttons->checkedId());
    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox);
}

void NoteAlarmDialog::setAlarm(const KDateTime &dateTime)
{
    if (dateTime.isValid()) {
        m_buttons->button(1)->setChecked(true);
        m_atDate->setDate(dateTime.date());
        m_atTime->setTime(dateTime.time());
    } else {
        m_buttons->button(0)->setChecked(true);
    }
    slotButtonChanged(m_buttons->checkedId());
}

void NoteAlarmDialog::slotButtonChanged(int id)
{
    switch (id) {
    case 0:
        m_atDate->setEnabled(false);
        m_atTime->setEnabled(false);
        break;
    case 1:
        m_atDate->setEnabled(true);
        m_atTime->setEnabled(true);
        break;
    }
}

KDateTime NoteAlarmDialog::alarm() const
{
    if (m_buttons->checkedId() == 1) {
        return KDateTime(m_atDate->date(), m_atTime->time(), KDateTime::LocalZone);
    } else {
        return KDateTime();
    }
}
