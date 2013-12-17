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


#include "webdavsettingsdialog.h"

#include <KLocalizedString>
#include <KLineEdit>

#include <QLabel>
#include <QVBoxLayout>

using namespace PimCommon;

WebDavSettingsDialog::WebDavSettingsDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "WebDav Settings" ) );
    setButtons( Ok | Cancel );
    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;

    QLabel *lab = new QLabel(i18n("Service Location (e.g. https://dav.example.com/)"));
    lay->addWidget(lab);

    mServiceLocation = new KLineEdit;
    lay->addWidget(mServiceLocation);

    lab = new QLabel(i18n("Public location (Optional)"));
    lay->addWidget(lab);

    mPublicLocation = new KLineEdit;
    lay->addWidget(mPublicLocation);

    w->setLayout(lay);
    setMainWidget(w);
    connect(mServiceLocation, SIGNAL(textChanged(QString)), this, SLOT(slotServiceLocationChanged(QString)));
    enableButtonOk(false);
}

WebDavSettingsDialog::~WebDavSettingsDialog()
{

}

void WebDavSettingsDialog::slotServiceLocationChanged(const QString &text)
{
    enableButtonOk(!text.isEmpty());
}

QString WebDavSettingsDialog::serviceLocation() const
{
    return mServiceLocation->text();
}

QString WebDavSettingsDialog::publicLocation() const
{
    return mPublicLocation->text();
}
