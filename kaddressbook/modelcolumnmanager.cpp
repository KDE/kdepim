/*
  This file is part of KAddressBook.

  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#include "modelcolumnmanager.h"
#include "settings.h"

#include <KContacts/Addressee>

#include <KLocalizedString>

#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QMenu>
#include <QWidget>

ModelColumnManager::ModelColumnManager(Akonadi::ContactsTreeModel *model, QObject *parent)
    : QObject(parent), mModel(model), mWidget(Q_NULLPTR)
{
}

void ModelColumnManager::load()
{
    const QList<int> settingsColumns = Settings::contactModelColumns();
    Akonadi::ContactsTreeModel::Columns columns;

    foreach (int column, settingsColumns) {
        columns.append((Akonadi::ContactsTreeModel::Column)column);
    }

    mModel->setColumns(columns);
}

void ModelColumnManager::store()
{
    const Akonadi::ContactsTreeModel::Columns columns = mModel->columns();
    QList<int> settingsColumns;

    foreach (int column, columns) {
        settingsColumns.append((int)column);
    }

    Settings::setContactModelColumns(settingsColumns);
}

void ModelColumnManager::setWidget(QWidget *widget)
{
    mWidget = widget;
    mWidget->installEventFilter(this);
}

bool ModelColumnManager::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == mWidget) {
        if (event->type() == QEvent::ContextMenu) {
            QMenu menu;

            Akonadi::ContactsTreeModel::Columns columns = mModel->columns();

            QAction *fullNameAction = menu.addAction(i18n("Full Name"));
            fullNameAction->setCheckable(true);
            fullNameAction->setChecked(
                columns.contains(Akonadi::ContactsTreeModel::FullName));
            fullNameAction->setEnabled(false);

            QAction *familyNameAction = menu.addAction(i18n("Family Name"));
            familyNameAction->setCheckable(true);
            familyNameAction->setChecked(
                columns.contains(Akonadi::ContactsTreeModel::FamilyName));

            QAction *givenNameAction = menu.addAction(i18n("Given Name"));
            givenNameAction->setCheckable(true);
            givenNameAction->setChecked(
                columns.contains(Akonadi::ContactsTreeModel::GivenName));

            QAction *birthdayAction = menu.addAction(KContacts::Addressee::birthdayLabel());
            birthdayAction->setCheckable(true);
            birthdayAction->setChecked(
                columns.contains(Akonadi::ContactsTreeModel::Birthday));

            QAction *homeAddressAction = menu.addAction(i18n("Home Address"));
            homeAddressAction->setCheckable(true);
            homeAddressAction->setChecked(
                columns.contains(Akonadi::ContactsTreeModel::HomeAddress));

            QAction *businessAddressAction = menu.addAction(i18n("Business Address"));
            businessAddressAction->setCheckable(true);
            businessAddressAction->setChecked(
                columns.contains(Akonadi::ContactsTreeModel::BusinessAddress));

            QAction *phoneNumbersAction = menu.addAction(i18n("Phone Numbers"));
            phoneNumbersAction->setCheckable(true);
            phoneNumbersAction->setChecked(
                columns.contains(Akonadi::ContactsTreeModel::PhoneNumbers));

            QAction *preferredEmailAction = menu.addAction(i18n("Preferred EMail"));
            preferredEmailAction->setCheckable(true);
            preferredEmailAction->setChecked(
                columns.contains(Akonadi::ContactsTreeModel::PreferredEmail));

            QAction *allEmailsAction = menu.addAction(i18n("All EMails"));
            allEmailsAction->setCheckable(true);
            allEmailsAction->setChecked(
                columns.contains(Akonadi::ContactsTreeModel::AllEmails));

            QAction *organizationAction = menu.addAction(KContacts::Addressee::organizationLabel());
            organizationAction->setCheckable(true);
            organizationAction->setChecked(
                columns.contains(Akonadi::ContactsTreeModel::Organization));

            QAction *roleAction = menu.addAction(KContacts::Addressee::roleLabel());
            roleAction->setCheckable(true);
            roleAction->setChecked(
                columns.contains(Akonadi::ContactsTreeModel::Role));

            QAction *homepageAction = menu.addAction(KContacts::Addressee::urlLabel());
            homepageAction->setCheckable(true);
            homepageAction->setChecked(
                columns.contains(Akonadi::ContactsTreeModel::Homepage));

            QAction *noteAction = menu.addAction(KContacts::Addressee::noteLabel());
            noteAction->setCheckable(true);
            noteAction->setChecked(
                columns.contains(Akonadi::ContactsTreeModel::Note));

            if (menu.exec(((QContextMenuEvent *)event)->globalPos())) {
                Akonadi::ContactsTreeModel::Columns columns;

                if (fullNameAction->isChecked()) {
                    columns << Akonadi::ContactsTreeModel::FullName;
                }
                if (familyNameAction->isChecked()) {
                    columns << Akonadi::ContactsTreeModel::FamilyName;
                }
                if (givenNameAction->isChecked()) {
                    columns << Akonadi::ContactsTreeModel::GivenName;
                }
                if (birthdayAction->isChecked()) {
                    columns << Akonadi::ContactsTreeModel::Birthday;
                }
                if (homeAddressAction->isChecked()) {
                    columns << Akonadi::ContactsTreeModel::HomeAddress;
                }
                if (businessAddressAction->isChecked()) {
                    columns << Akonadi::ContactsTreeModel::BusinessAddress;
                }
                if (phoneNumbersAction->isChecked()) {
                    columns << Akonadi::ContactsTreeModel::PhoneNumbers;
                }
                if (preferredEmailAction->isChecked()) {
                    columns << Akonadi::ContactsTreeModel::PreferredEmail;
                }
                if (allEmailsAction->isChecked()) {
                    columns << Akonadi::ContactsTreeModel::AllEmails;
                }
                if (organizationAction->isChecked()) {
                    columns << Akonadi::ContactsTreeModel::Organization;
                }
                if (roleAction->isChecked()) {
                    columns << Akonadi::ContactsTreeModel::Role;
                }
                if (homepageAction->isChecked()) {
                    columns << Akonadi::ContactsTreeModel::Homepage;
                }
                if (noteAction->isChecked()) {
                    columns << Akonadi::ContactsTreeModel::Note;
                }

                mModel->setColumns(columns);
                QTimer::singleShot(0, this, SLOT(adaptHeaderView()));
            }

            return true;
        } else {
            return false;
        }
    }

    return false;
}

void ModelColumnManager::adaptHeaderView()
{
    QHeaderView *view = qobject_cast<QHeaderView *>(mWidget);
    if (view) {
        view->resizeSections(QHeaderView::Stretch);

        view->setDefaultAlignment(mModel->columns().count() == 1 ? Qt::AlignCenter : Qt::AlignLeft);
    }
}

