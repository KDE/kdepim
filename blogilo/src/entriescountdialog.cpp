/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#include "entriescountdialog.h"
#include "settings.h"

#include <qdebug.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

EntriesCountDialog::EntriesCountDialog( QWidget *parent )
    : QDialog( parent )
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QWidget *widget = new QWidget;
    mainLayout->addWidget(widget);
    ui.setupUi( widget );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(buttonBox);
    ui.spinCount->setValue(Settings::updateEntriesCount());
}

EntriesCountDialog::~EntriesCountDialog()
{
    Settings::setShowUpdateEntriesDialog( !ui.kcfg_remember->isChecked() );
    Settings::setUpdateEntriesCount( ui.spinCount->value() );
    Settings::self()->save();
}

int EntriesCountDialog::count() const
{
    return ui.spinCount->value();
}

