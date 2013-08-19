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

SelectThunderbirdProfileDialog::SelectThunderbirdProfileDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Select thunderbird profile" ) );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );
    QWidget *mainWidget = new QWidget( this );
    QHBoxLayout *mainLayout = new QHBoxLayout( mainWidget );
    mainLayout->setSpacing( KDialog::spacingHint() );
    mainLayout->setMargin( KDialog::marginHint() );
    setMainWidget( mainWidget );
    mSelectProfile = new SelectThunderbirdProfileWidget(mainWidget);
    mainLayout->addWidget(mSelectProfile);
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

#include "selectthunderbirdprofilewidget.moc"
