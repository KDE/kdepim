/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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
#include "selectcomponentpage.h"
#include "ui_selectcomponentpage.h"

SelectComponentPage::SelectComponentPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectComponentPage)
{
    ui->setupUi(this);
    connect( ui->everything, SIGNAL(clicked(bool)), this, SLOT(slotEverythingClicked(bool)) );
    connect( ui->addressbooks, SIGNAL(clicked(bool)), this, SLOT(slotComponentClicked()) );
    connect( ui->filters, SIGNAL(clicked(bool)), this, SLOT(slotComponentClicked()) );
    connect( ui->mails, SIGNAL(clicked(bool)), this, SLOT(slotComponentClicked()) );
    connect( ui->settings, SIGNAL(clicked(bool)), this, SLOT(slotComponentClicked()) );
    connect( ui->calendars, SIGNAL(clicked(bool)), this, SLOT(slotComponentClicked()) );
}

SelectComponentPage::~SelectComponentPage()
{
    delete ui;
}

void SelectComponentPage::slotComponentClicked()
{
    const bool componentSelected = ( ui->addressbooks->isChecked() || ui->filters->isChecked() || ui->mails->isChecked() || ui->settings->isChecked()|| ui->calendars->isChecked() || ui->everything->isChecked() );
    Q_EMIT atLeastOneComponentSelected(componentSelected);
}

void SelectComponentPage::slotEverythingClicked( bool clicked )
{
    ui->addressbooks->setEnabled( !clicked && (mOptions & AbstractImporter::AddressBooks));
    ui->filters->setEnabled( !clicked && (mOptions & AbstractImporter::Filters));
    ui->mails->setEnabled( !clicked && (mOptions & AbstractImporter::Mails));
    ui->settings->setEnabled( !clicked && (mOptions & AbstractImporter::Settings));
    ui->calendars->setEnabled( !clicked && (mOptions & AbstractImporter::Calendars));
    slotComponentClicked();
}


void SelectComponentPage::setEnabledComponent(AbstractImporter::TypeSupportedOptions options)
{
    mOptions = options;
    slotEverythingClicked(ui->everything->isChecked());
}

AbstractImporter::TypeSupportedOptions SelectComponentPage::selectedComponents() const
{
    if ( ui->everything->isChecked() ) {
        return mOptions;
    } else {
        AbstractImporter::TypeSupportedOptions newOptions;
        if (ui->addressbooks->isChecked()) {
            newOptions|=AbstractImporter::AddressBooks;
        }
        if (ui->filters->isChecked()) {
            newOptions|=AbstractImporter::Filters;
        }
        if (ui->mails->isChecked()) {
            newOptions|=AbstractImporter::Mails;
        }
        if (ui->settings->isChecked()) {
            newOptions|=AbstractImporter::Settings;
        }
        if (ui->calendars->isChecked()) {
            newOptions|=AbstractImporter::Calendars;
        }

        return newOptions;
    }
}

#include "selectcomponentpage.moc"
