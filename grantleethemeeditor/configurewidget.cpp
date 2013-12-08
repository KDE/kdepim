/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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


#include "configurewidget.h"
#include "ui_configurewidget.h"
#include "globalsettings_base.h"

#include <KSharedConfig>

using namespace GrantleeThemeEditor;
ConfigureWidget::ConfigureWidget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::ConfigureWidget)
{
    ui->setupUi(this);
}

ConfigureWidget::~ConfigureWidget()
{
    delete ui;
}

void ConfigureWidget::writeConfig()
{
    GrantleeThemeEditor::GrantleeThemeEditorSettings::setAuthorEmail(ui->authorEmail->text());
    GrantleeThemeEditor::GrantleeThemeEditorSettings::setAuthor(ui->author->text());
    GrantleeThemeEditor::GrantleeThemeEditorSettings::setPath(ui->defaultPath->url().path());
    GrantleeThemeEditor::GrantleeThemeEditorSettings::self()->writeConfig();
}

void ConfigureWidget::readConfig()
{
    ui->authorEmail->setText(GrantleeThemeEditor::GrantleeThemeEditorSettings::authorEmail());
    ui->author->setText(GrantleeThemeEditor::GrantleeThemeEditorSettings::author());
    ui->defaultPath->setUrl(KUrl(GrantleeThemeEditor::GrantleeThemeEditorSettings::path()));
}

void ConfigureWidget::setDefault()
{
    ui->defaultPath->setUrl(KUrl());
    ui->authorEmail->clear();
    ui->author->clear();
}


