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

#include "firstpage.h"
#include <libkmobiletools/engineslist.h>
#include <libkmobiletools/engine.h>
#include <libkmobiletools/devicesconfig.h>

#define NEXTPAGE 2

#include <kmessagebox.h>
#include <QAbstractButton>

using namespace KMobileTools;
class FirstPagePrivate {
public:
    FirstPagePrivate() : hasEngines(false)
    {};
    QString enginelibname;
    bool hasEngines;
    QList<QWizardPage*> enginePages;
};

FirstPage::FirstPage(QWidget *parent)
    : QWizardPage(parent)
{
    d=new FirstPagePrivate;
    setupUi(this);
    setTitle(i18nc("First new device wizard page title", "Mobile Phone Informations") );
    setCommitPage(true);
}

QString FirstPage::engineLibrary() const {
    return d->enginelibname;
}

void FirstPage::engineSelected(int index)
{
    kDebug() << "FirstPage::engineSelected(" << index << ")\n";
    KPluginInfo *engInfo=KMobileTools::EnginesList::instance()->engineInfo(engineSelection->itemData(index).toString() );
    if(!engInfo)
    {
        kDebug() << "No data found for " << index << endl;
        KMobileTools::EnginesList::instance()->setWizardEngine();
        d->enginelibname.clear();
        emit engineLibraryChanged( engineLibrary() );
        return;
    }
    engineDescLabel->setText(engInfo->property("Description").toString().replace('\n',"<br>") );
//     setNextEnabled ( currentPage(), true );
    d->enginelibname=engInfo->service()->library();
    emit engineLibraryChanged( engineLibrary() );
//     setField("engine",d->enginelibname);
    // now let's load the engine library
    kDebug() << "Engine Library field changed: " << field("engine").toString() << endl;
    KMobileTools::Engine *engine=KMobileTools::Engine::load( engineLibrary() , wizard() );
    if(!engine)
    {
        KMessageBox::error(this, i18n("Could not load the engine %1.\nIf this error persists, please restart KMobileTools.",
                           engInfo->name() ) );
        KMobileTools::EnginesList::instance()->setWizardEngine();
        return;
    }
    KMobileTools::EnginesList::instance()->setWizardEngine(engine);
    for (int i=0; i<d->enginePages.size(); i++)
        delete d->enginePages.at(i); // deleting old QWizardPages
    d->enginePages=KMobileTools::EnginesList::instance()->wizardEngine()->wizardPages(this->wizard());
    for(int i=0; i<d->enginePages.size(); i++)
        wizard()->setPage(NEXTPAGE+i, d->enginePages.at(i));
}

void FirstPage::slotCompleteChanged()
{
    kDebug() << "FirstPage::slotCompleteChanged(); is complete: " << isComplete() << endl;
    if(wizard()) wizard()->button(QWizard::CommitButton)->setEnabled(isComplete() );
}


void FirstPage::initializePage()
{
    kDebug() << "FirstPage::initializePage()" << endl;
    connect(this, SIGNAL(completeChanged()), this, SLOT(slotCompleteChanged()));
    registerField("phonename*", phonename);
    registerField("engine*", this, "engineLibrary", SIGNAL(engineLibraryChanged(const QString&)) );
    // detecting engines
    KPluginInfo::List engines=KMobileTools::EnginesList::instance()->availEngines();
    if(engines.count())
    {
        d->hasEngines=true;
        for(int i=0; i<engines.count(); i++ )
            // Adding libname as userdata
            engineSelection->addItem(engines[i]->name(), engines[i]->service()->library() );
        engineSelected(engineSelection->currentIndex());
    }
    else
    {
        engineSelection->addItem(i18n("No engines found. Reinstall KMobileTools") );
//         setNextEnabled(currentPage(), false); @TODO port to new QWizard API
    }
    kDebug() << "is complete? " << isComplete() << endl;
    connect(engineSelection, SIGNAL(currentIndexChanged (int)), this, SLOT(engineSelected(int)) );
}

bool FirstPage::isFinalPage() const { return false; }
bool FirstPage::validatePage() {
    if(!EnginesList::instance()->wizardEngine()) return false; // just for safety
    kDebug() << "Creating config entry named " << wizard()->objectName() << ";" << endl;
    KMobileTools::DevicesConfig *cfg=EnginesList::instance()->wizardEngine()->config(true, wizard()->objectName() );
    cfg->setDevicename(field("phonename").toString() );
    cfg->setEngine(field("engine").toString() );
    cfg->writeConfig();
    delete cfg;
    return QWizardPage::validatePage();
}


#include "firstpage.moc"
