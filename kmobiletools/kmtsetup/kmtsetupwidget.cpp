/***************************************************************************
   Copyright (C) 2007
   by Davide Bettio <davide.bettio@kdemail.net>
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "kmtsetupwidget.h"

#include <qlabel.h>
#include <qstring.h>
#include <qdir.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3ValueList>
#include <k3listview.h>
#include <qstringlist.h>
#include <k3process.h>
#include <kglobal.h>
#include <kuser.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kstandarddirs.h>



KMTSetupWidget::KMTSetupWidget(QWidget* parent, const char* name)
    : KDialogBase(parent,name,true, i18n("KMobileTools Permission Wizard"), Ok | Cancel | Default )
{
    KMessageBox::information( this, i18n("Welcome to the KMobileTools Permission Wizard.\nHere you can fix the permissions to let KMobileTools communicate with your devices.\nPlease connect all your mobile phones to your PC now."));
    p_widget=new KMTSetupWidgetBase(this, name);
    setMainWidget(p_widget);
    loadGroupsLists();
    loadUsersLists();
    connect(p_widget->btnAddDevice, SIGNAL(clicked()), this, SLOT(btnAddDevice_clicked()));
    connect(p_widget->lstUsers, SIGNAL(clicked(Q3ListViewItem*)), this, SLOT(userListClicked( Q3ListViewItem *)));
    connect(p_widget->remUser, SIGNAL(clicked()), this, SLOT(remUserClicked()));
    QPixmap wizardLogoPixmap;
    wizardLogoPixmap.load( KGlobal::dirs ()->findResource("data", "kmtsetup/kmobilewizard.png") );
    p_widget->wizardLogo->setPixmap( wizardLogoPixmap );
}

KMTSetupWidget::~KMTSetupWidget(){
}

void KMTSetupWidget::loadGroupsLists(){
    p_widget->lstDevices->clear();

    QDir devices("/dev/");
    devices.setFilter(QDir::System);
    devices.setNameFilter("rfcomm* ttyS* ttyACM* ttyUSB* ircomm*");

    const QFileInfoList *devlist = devices.entryInfoList();
    QFileInfoListIterator devicesiterator(*devlist);
    QFileInfo *deviceinfo;

    QString devicegroup;
    bool first;

    while ( (deviceinfo = devicesiterator.current()) != 0 ){

        devicegroup = deviceinfo->group();

        first = true;

        for (unsigned int i = 0; i < groupslist.count(); ++i){
            if (devicegroup == *groupslist.at(i)) first = false;
        }
        if (first == true){
            groupslist.append(devicegroup);
        }

        (void) new K3ListViewItem(p_widget->lstDevices, deviceinfo->fileName() ,"/dev/" + deviceinfo->fileName(), deviceinfo->owner() + ' ' + devicegroup);
        ++devicesiterator;
    };
    deviceinfo=new QFileInfo("/var/lock");
    (void) new K3ListViewItem(p_widget->lstDevices, deviceinfo->fileName(), "/var/lock/", deviceinfo->owner() + ' ' + devicegroup);
    delete deviceinfo;
}

void KMTSetupWidget::loadUsersLists(){
    p_widget->lstUsers->clear();

    QString groups = "";
    for (unsigned int i = 0; i < groupslist.count(); ++i) groups.append(*groupslist.at(i) + ' ');

    Q3ValueList<KUser> ulist = KUser::allUsers();

    KUser user;

    for (unsigned int i = 0; i < ulist.count(); i++){

        user = *ulist.at(i);

        if (user.uid() >= 1000 && user.loginName() != "nobody"){

            userslist.append(user.loginName());

            (void) new K3ListViewItem(p_widget->lstUsers, user.loginName(), groups);
        }
    }
}

void KMTSetupWidget::addUserToGroup(const QString &user, const QString &group){
    //gpasswd -a user group

    K3Process *gpasswdproc = new K3Process();
    *gpasswdproc << "gpasswd";
    *gpasswdproc << "-a" << user.trimmed() << group.trimmed();
    kDebug() << "Process arguments: " << gpasswdproc->args() << endl;
    gpasswdproc->start(K3Process::Block);
}

void KMTSetupWidget::slotDefault(){
    loadGroupsLists();
    loadUsersLists();
}
void KMTSetupWidget::btnAddDevice_clicked(){
    QString device=KInputDialog::text(i18n("New Device"), i18n("Add here the path to the device you want to chech permissions."), QString(), 0, this);
    if(device.isNull() ) return;
    if(!QFile::exists(device))
    {
        kDebug() << "Device doesn't exist: <<" << device << ">>\n";
        KMessageBox::error(this, i18n("You must enter a valid directory or filename."));
        return;
    }
    QFileInfo fi(device);
    groupslist+=fi.group();
    (void) new K3ListViewItem(p_widget->lstDevices, fi.fileName() , fi.filePath(), fi.owner() + ' ' + fi.group() );
}

void KMTSetupWidget::slotOk(){
    QString groups = "";
    kDebug() << "grouplist:" << groupslist << endl;

    for (unsigned int i = 0; i < userslist.count(); ++i){
        for(QStringList::Iterator it=groupslist.begin(); it!=groupslist.end(); ++it)
            addUserToGroup(*userslist.at(i), *it);
    }
    KMessageBox::information( this, i18n("All changes are done. Please remember to logout and login back for applying changes."));
    KDialogBase::slotOk();
}

void KMTSetupWidget::slotCancel(){
    parentWidget() ? parentWidget()->close() : close();
    KDialogBase::slotCancel();
}


#include "kmtsetupwidget.moc"



/*!
    \fn KMTSetupWidget::remUserClicked()
 */
void KMTSetupWidget::remUserClicked()
{
    if(!p_widget->lstUsers->selectedItem()) return;
    userslist.remove( p_widget->lstUsers->selectedItem()->text(0));
    delete p_widget->lstUsers->selectedItem();
    p_widget->remUser->setEnabled(false);
}


/*!
    \fn KMTSetupWidget::userListClicked( QListViewItem * item )
 */
void KMTSetupWidget::userListClicked( Q3ListViewItem * item )
{
    if(!item)
    {
        p_widget->remUser->setEnabled(false);
        return;
    }
    p_widget->remUser->setEnabled(true);
}

