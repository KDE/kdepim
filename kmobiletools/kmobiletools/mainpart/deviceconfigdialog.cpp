/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

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
#include "deviceconfigdialog.h"

#include <kmessagebox.h>
#include <klocale.h>
#include <klineedit.h>
#include <kconfigskeleton.h>
#include <q3buttongroup.h>
#include <kdebug.h>
#include <knuminput.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <kapplication.h>
#include <qtabwidget.h>
#include <kpushbutton.h>
#include <math.h>
#include <keditlistbox.h>
#include <k3listview.h>
#include <kstandarddirs.h>
#include <qstringlist.h>
#include <q3widgetstack.h>
#include <kplugininfo.h>

#include <libkmobiletools/engineslist.h>
#include <libkmobiletools/engine.h>
#include <libkmobiletools/kmobiletoolshelper.h>

#include "ui_wizDeviceFirst.h"

#include "ui_genericDeviceOptions.h"
// #include "deviceIFace_stub.h" @TODO update to dbus if necessary
#include <picksmscenter.h>

#include "ui_cfg_filesystem.h"
#include "kcombobox.h"

#define CONN_USB            1
#define CONN_USB_ID         0
#define CONN_IRDA           2
#define CONN_IRDA_ID        1
#define CONN_BLUETOOTH      4
#define CONN_BLUETOOTH_ID   2
#define CONN_SERIAL         8
#define CONN_SERIAL_ID      3

using namespace KMobileTools;

deviceConfigDialog::deviceConfigDialog(QWidget *parent, const QString &name, KConfigSkeleton *config)
    : KConfigDialog(parent, name, config )
{
     setButtons(Ok|Apply|Cancel);
     setModal(true);
     setDefaultButton(Ok);
     setFaceType(List);
w_firstPage=new QWidget();
w_genOptions=new QWidget();
w_fsConfig=new QWidget();
    firstPage=new Ui::wizDeviceFirst(); firstPage->setupUi(w_firstPage );
    firstPage->kcfg_engine->hide(); // @TODO fix this ugly hack
    KPluginInfo::List engines=KMobileTools::EnginesList::instance()->availEngines();
    for(int i=0; i<engines.count(); i++)
        firstPage->combo_engine->addItem(engines[i].name(), engines[i].service()->library() );

#ifdef ENABLE_FS
    fsConfig=new Ui::cfgFilesystem(); fsConfig->setupUi(w_fsConfig());
#else
    fsConfig=new QLabel(i18n("Support for filesystem is currently disabled. To reenable it, look KMobileTools documentation on our homepage, and run configure with the correct flags."), NULL);
    fsConfig->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    fsConfig->setWordWrap(true);
#endif

#ifdef ENABLE_FS
    #ifndef HAVE_OBEXFTP
        fsConfig->kcfg_fstype->changeItem(fsConfig->kcfg_fstype->text(2).append(i18n(" - not compiled")), 2);
    #endif
    #ifndef HAVE_P2KLIB
        fsConfig->kcfg_fstype->changeItem(fsConfig->kcfg_fstype->text(1).append(i18n(" - not compiled")), 1);
    #endif
#endif
    genOptions=new Ui::genericDeviceOptions(); genOptions->setupUi(w_genOptions);
    addPage(w_firstPage, i18n("Name and type"), QString("blockdevice") );
    addPage(w_genOptions, i18n("Device Options"), "kmobiletools");
//     addPage(w_at_engine, i18n("AT Engine"), "kmobiletools");
    addPage(w_fsConfig, i18n("Files Access"), "folder_yellow");

    connect(firstPage->combo_engine, SIGNAL(activated(const QString &) ), this, SLOT(slotEngineChanged(const QString & ) ) );
    connect(firstPage->kcfg_engine, SIGNAL(textChanged ( const QString & )), this, SLOT(readEngine(const QString&) ) );
    connect(genOptions->kcfg_status_poll, SIGNAL(toggled(bool) ), this, SLOT(slotPollEnabled(bool ) ) );
    connect(genOptions->b_smscenter_choose, SIGNAL(clicked()), this, SLOT(chooseSMSCenter()));
#ifdef ENABLE_FS
    connect(fsConfig->kcfg_fstype, SIGNAL(activated(int)), this, SLOT(fs_selected( int )));
#endif
    genOptions->kcfg_verbose->setText(i18nc("debug option in device configuration dialog",
        "&Verbose output in %1 (for debug)", KGlobal::dirs()->saveLocation("tmp", "kmobiletools", true)) );
    slotEngineChanged( DEVCFG(name)->engine() );
    slotPollEnabled( DEVCFG(name)->status_poll() );

    QStringList sl_slots;

    fs_selected(DEVCFG(name)->fstype());
    updateWidgets();
}

void deviceConfigDialog::readEngine(const QString &enginename)
{
    int key=firstPage->combo_engine->findData(enginename);
    if (key==-1) return;
    firstPage->combo_engine->setCurrentIndex(key);
}


deviceConfigDialog::~deviceConfigDialog()
{
}

void deviceConfigDialog::fs_selected(int item)
{
    kDebug() <<"Filesystem ID:"<< item;
#ifdef ENABLE_FS
    fsConfig->fs_stack->raiseWidget(item);
#endif
}


#include "deviceconfigdialog.moc"

void deviceConfigDialog::updateSettings()
{
    KConfigDialog::updateSettings();
}

void deviceConfigDialog::slotOk()
{
    slotApply();
    done(QDialog::Accepted);
}
void deviceConfigDialog::slotApply()
{
    if( ! firstPage->kcfg_devicename->text().length() )
        KMessageBox::error(this, i18n("You must enter a name for your device"), i18n("Error") );
}


/*!
    \fn deviceConfigDialog::slotEngineChanged(int id)
 */
void deviceConfigDialog::slotEngineChanged(const QString &enginename)
{
    QString runningEngine=firstPage->combo_engine->itemData(firstPage->combo_engine->currentIndex() ).toString();
    firstPage->kcfg_engine->setText( runningEngine );
    kDebug() <<"Changed engine:" << runningEngine;
    if( currentEngine==runningEngine )
    {
        if(enginepages.count()) return;
        KMessageBox::information(this, i18n("<qt><p>After changing the mobile phone engine you must reload the configuration dialog.</p><p>This window will now close.</p></qt>"));
        done(QDialog::Accepted);
    }
    currentEngine=runningEngine;
    KMobileTools::Engine *engine=KMobileTools::Engine::load(runningEngine);
    enginepages=engine->configWidgets(this);
    for(int i=0; i<enginepages.count(); i++)
        addPage(enginepages.value(i),
                enginepages.value(i)->property("itemName").toString(),
                enginepages.value(i)->property("pixmapName").toString(),
                enginepages.value(i)->property("header").toString() );
    delete engine;
}

void deviceConfigDialog::slotPollEnabled(bool poll)
{
    if(poll)
    {
        genOptions->poll2Label->setEnabled(true);
        genOptions->kcfg_status_pollTimeout->setEnabled(true);
        return;
    }
    genOptions->poll2Label->setEnabled(false);
    genOptions->kcfg_status_pollTimeout->setEnabled(false);
}




/*!
    \fn deviceConfigDialog::chooseSMSCenter()
 */
void deviceConfigDialog::chooseSMSCenter()
{
    PickSMSCenter *dlg=new PickSMSCenter(this);
    dlg->exec();
    if(!dlg->smsCenter().isNull() ) genOptions->kcfg_smscenter->setText(dlg->smsCenter() );
}
