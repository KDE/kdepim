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

#include "shorturlconfiguredialog.h"
#include "shorturlconfigurewidget.h"

#include <KLocale>
#include <KSeparator>
#include <QVBoxLayout>

using namespace PimCommon;
ShortUrlConfigureDialog::ShortUrlConfigureDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Configure engine" ) );
    setButtons( Cancel | Ok | Default );

    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);
    setMainWidget(w);
    mConfigureWidget = new ShortUrlConfigureWidget();
    mConfigureWidget->loadConfig();
    lay->addWidget(mConfigureWidget);
    lay->addWidget(new KSeparator);
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));
    connect(this, SIGNAL(defaultClicked()), this, SLOT(slotDefaultClicked()));
}


ShortUrlConfigureDialog::~ShortUrlConfigureDialog()
{

}

void ShortUrlConfigureDialog::slotOkClicked()
{
    mConfigureWidget->writeConfig();
    accept();
}

void ShortUrlConfigureDialog::slotDefaultClicked()
{
    mConfigureWidget->resetToDefault();
}

#include "moc_shorturlconfiguredialog.cpp"
