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


#include "gravatarcreatewidget.h"

#include <QLabel>
#include <QGridLayout>
using namespace KABGravatar;

GravatarCreateWidget::GravatarCreateWidget(QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *mainLayout = new QGridLayout;
    setLayout(mainLayout);
    //KF5 add i18n
    QLabel *lab = new QLabel(QLatin1String("Email:"));
    lab->setObjectName(QLatin1String("emaillabel"));
    mainLayout->addWidget(lab, 0, 0);

    mEmailLab = new QLabel;
    mEmailLab->setObjectName(QLatin1String("email"));
    mainLayout->addWidget(mEmailLab, 0, 1);
}


GravatarCreateWidget::~GravatarCreateWidget()
{

}

void GravatarCreateWidget::setEmail(const QString &email)
{
    if (mEmail != email) {
        mEmail = email;
        mEmailLab->setText(mEmail);
    }
}
