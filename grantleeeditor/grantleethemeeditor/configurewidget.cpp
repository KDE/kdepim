/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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
#include <QUrl>

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
    const QString authorEmail = ui->authorEmail->text().trimmed();
    if (!authorEmail.isEmpty())
        GrantleeThemeEditor::GrantleeThemeEditorSettings::setAuthorEmail(authorEmail);
    const QString authorName = ui->author->text().trimmed();
    if (!authorName.isEmpty())
        GrantleeThemeEditor::GrantleeThemeEditorSettings::setAuthor(authorName);
    QUrl url = ui->defaultPath->url();
    if (url.isValid())
        GrantleeThemeEditor::GrantleeThemeEditorSettings::setPath(url.path());
    GrantleeThemeEditor::GrantleeThemeEditorSettings::self()->save();
}

void ConfigureWidget::readConfig()
{
    ui->authorEmail->setText(GrantleeThemeEditor::GrantleeThemeEditorSettings::authorEmail());
    ui->author->setText(GrantleeThemeEditor::GrantleeThemeEditorSettings::author());
    ui->defaultPath->setUrl(QUrl::fromLocalFile(GrantleeThemeEditor::GrantleeThemeEditorSettings::path()));
}

void ConfigureWidget::setDefault()
{
    ui->defaultPath->setUrl(QUrl());
    ui->authorEmail->clear();
    ui->author->clear();
}

