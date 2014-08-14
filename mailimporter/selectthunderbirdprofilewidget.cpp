/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
  
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

#include "selectthunderbirdprofilewidget.h"
#include "ui_selectthunderbirdprofilewidget.h"

#include <QHBoxLayout>
#include <KLocalizedString>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

SelectThunderbirdProfileDialog::SelectThunderbirdProfileDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle( i18n( "Select thunderbird profile" ) );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QVBoxLayout *topLayout = new QVBoxLayout;
    setLayout(topLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SelectThunderbirdProfileDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SelectThunderbirdProfileDialog::reject);
    okButton->setDefault(true);
    setModal( true );
//TODO PORT QT5     mainLayout->setSpacing( QDialog::spacingHint() );
//TODO PORT QT5     mainLayout->setMargin( QDialog::marginHint() );
    mSelectProfile = new SelectThunderbirdProfileWidget(this);
    topLayout->addWidget(mSelectProfile);
    topLayout->addWidget(buttonBox);
}

SelectThunderbirdProfileDialog::~SelectThunderbirdProfileDialog()
{

}

void SelectThunderbirdProfileDialog::fillProfile(const QMap<QString,QString> &map, const QString &defaultProfile)
{
    mSelectProfile->fillProfile(map,defaultProfile);
}

QString SelectThunderbirdProfileDialog::selectedProfile() const
{
    return mSelectProfile->selectedProfile();
}


SelectThunderbirdProfileWidget::SelectThunderbirdProfileWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectThunderbirdProfileWidget)
{
    ui->setupUi(this);
}

SelectThunderbirdProfileWidget::~SelectThunderbirdProfileWidget()
{
    delete ui;
}

void SelectThunderbirdProfileWidget::fillProfile(const QMap<QString,QString> &map, const QString &defaultProfile)
{
    QMap<QString, QString>::const_iterator i = map.constBegin();
    while (i != map.constEnd()) {
        QString name = i.key();
        if(i.value()==defaultProfile){
            name+=i18n(" (default)");
        }
        ui->profile->addItem(name,i.value());
        ++i;
    }
}

QString SelectThunderbirdProfileWidget::selectedProfile() const
{
    return ui->profile->itemData(ui->profile->currentIndex()).toString();
}

