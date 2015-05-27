/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "gravatarupdatedialog.h"
#include "gravatarupdatewidget.h"
#include <QVBoxLayout>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KGlobal>
#include <KSharedConfig>
#include <QDialogButtonBox>

using namespace KABGravatar;

GravatarUpdateDialog::GravatarUpdateDialog(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mGravatarUpdateWidget = new GravatarUpdateWidget(this);
    mGravatarUpdateWidget->setObjectName(QLatin1String("gravatarupdatewidget"));
    mainLayout->addWidget(mGravatarUpdateWidget);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    buttonBox->setObjectName(QLatin1String("buttonbox"));
    mainLayout->addWidget(buttonBox);
    readConfig();
}

GravatarUpdateDialog::~GravatarUpdateDialog()
{
    writeConfig();
}

void GravatarUpdateDialog::setEmail(const QString &email)
{
    mGravatarUpdateWidget->setEmail(email);
}

void GravatarUpdateDialog::readConfig()
{
    KConfigGroup grp( KGlobal::config(), "GravatarUpdateDialog" );
    const QSize size = grp.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void GravatarUpdateDialog::writeConfig()
{
    KConfigGroup grp( KGlobal::config(), "GravatarUpdateDialog");
    grp.writeEntry( "Size", size() );
    grp.sync();
}
