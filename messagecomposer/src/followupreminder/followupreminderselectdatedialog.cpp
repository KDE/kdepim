/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "followupreminderselectdatedialog.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KDatePicker>
#include <KMessageBox>
#include <KDateComboBox>

#include <AkonadiWidgets/CollectionComboBox>

#include <KCalCore/Todo>

#include <QVBoxLayout>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <kdatecombobox.h>
using namespace MessageComposer;
class MessageComposer::FollowUpReminderSelectDateDialogPrivate
{
public:
    FollowUpReminderSelectDateDialogPrivate()
        : mDateComboBox(Q_NULLPTR),
          mCollectionCombobox(Q_NULLPTR),
          mOkButton(Q_NULLPTR)
    {

    }
    KDateComboBox *mDateComboBox;
    Akonadi::CollectionComboBox *mCollectionCombobox;
    QPushButton *mOkButton;
};

FollowUpReminderSelectDateDialog::FollowUpReminderSelectDateDialog(QWidget *parent, QAbstractItemModel *model)
    : QDialog(parent),
      d(new MessageComposer::FollowUpReminderSelectDateDialogPrivate)
{
    setWindowTitle(i18n("Select Date"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *topLayout = new QVBoxLayout;
    setLayout(topLayout);
    d->mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    d->mOkButton->setObjectName(QStringLiteral("ok_button"));
    d->mOkButton->setDefault(true);
    d->mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &FollowUpReminderSelectDateDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FollowUpReminderSelectDateDialog::reject);
    d->mOkButton->setDefault(true);
    setModal(true);

    QWidget *mainWidget = new QWidget(this);
    topLayout->addWidget(mainWidget);
    topLayout->addWidget(buttonBox);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    QFormLayout *formLayout = new QFormLayout;
    mainLayout->addLayout(formLayout);

    d->mDateComboBox = new KDateComboBox;
    d->mDateComboBox->setMinimumDate(QDate::currentDate());
    d->mDateComboBox->setObjectName(QStringLiteral("datecombobox"));

    QDate currentDate = QDate::currentDate();
    currentDate = currentDate.addDays(1);
    d->mDateComboBox->setDate(currentDate);

    formLayout->addRow(i18n("Date:"), d->mDateComboBox);

    d->mCollectionCombobox = new Akonadi::CollectionComboBox(model);
    d->mCollectionCombobox->setMinimumWidth(250);
    d->mCollectionCombobox->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    d->mCollectionCombobox->setMimeTypeFilter(QStringList() << KCalCore::Todo::todoMimeType());
    d->mCollectionCombobox->setObjectName(QStringLiteral("collectioncombobox"));

    formLayout->addRow(i18n("Store ToDo in:"), d->mCollectionCombobox);

    connect(d->mDateComboBox->lineEdit(), &QLineEdit::textChanged, this, &FollowUpReminderSelectDateDialog::slotDateChanged);
    connect(d->mCollectionCombobox, SIGNAL(currentIndexChanged(int)), SLOT(updateOkButton()));
    updateOkButton();
}

FollowUpReminderSelectDateDialog::~FollowUpReminderSelectDateDialog()
{
    delete d;
}

void FollowUpReminderSelectDateDialog::updateOkButton()
{
    d->mOkButton->setEnabled(!d->mDateComboBox->lineEdit()->text().isEmpty()
                             && d->mDateComboBox->date().isValid()
                             && (d->mCollectionCombobox->count() > 0)
                             && d->mCollectionCombobox->currentCollection().isValid());
}

void FollowUpReminderSelectDateDialog::slotDateChanged()
{
    updateOkButton();
}

QDate FollowUpReminderSelectDateDialog::selectedDate() const
{
    return d->mDateComboBox->date();
}

Akonadi::Collection FollowUpReminderSelectDateDialog::collection() const
{
    return d->mCollectionCombobox->currentCollection();
}

void FollowUpReminderSelectDateDialog::accept()
{
    const QDate date = selectedDate();
    if (date <= QDate::currentDate()) {
        KMessageBox::error(this, i18n("The selected date must be greater than the current date."), i18n("Invalid date"));
        return;
    }
    if (!d->mCollectionCombobox->currentCollection().isValid()) {
        KMessageBox::error(this, i18n("The selected folder is not valid."), i18n("Invalid folder"));
        return;
    }
    QDialog::accept();
}

