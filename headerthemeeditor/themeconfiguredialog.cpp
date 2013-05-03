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

#include "themeconfiguredialog.h"

#include <KLocale>
#include <KUrlRequester>
#include <KConfig>

#include <QVBoxLayout>
#include <QLabel>

ThemeConfigureDialog::ThemeConfigureDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Configure" ) );
    setButtons( Ok|Cancel );
    setButtonFocus( Ok );
    QWidget *w = new QWidget;

    QVBoxLayout *lay = new QVBoxLayout;
    w->setLayout(lay);

    QHBoxLayout *hbox = new QHBoxLayout;
    lay->addLayout(hbox);

    QLabel *lab = new QLabel(i18n("Default theme path:"));
    hbox->addWidget(lab);

    mDefaultUrl = new KUrlRequester;
    hbox->addWidget(mDefaultUrl);

    setMainWidget(w);
}

ThemeConfigureDialog::~ThemeConfigureDialog()
{
}

void ThemeConfigureDialog::loadConfig()
{
    //TODO
}

void ThemeConfigureDialog::writeConfig()
{
    //TODO
}

#include "themeconfiguredialog.moc"
