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

#include "kcmpimactivity.h"
#include "pimactivity/activitymanager.h"
#include "pimactivity/configureactivitywidget.h"

#include <KAboutData>
#include <kgenericfactory.h>
#include <KDialog>

#include <QVBoxLayout>

K_PLUGIN_FACTORY( KCMPimActivityFactory, registerPlugin<KCMPimActivity>(); )
K_EXPORT_PLUGIN( KCMPimActivityFactory( "kcmpimactivity" ) )


KCMPimActivity::KCMPimActivity(QWidget *parent, const QVariantList &args)
    : KCModule( KCMPimActivityFactory::componentData(), parent )
{
    KAboutData *about = new KAboutData( I18N_NOOP( "kcmpimactivity" ), 0,
                                        ki18n( "PIM Activity Settings" ),
                                        0, KLocalizedString(), KAboutData::License_LGPL,
                                        ki18n( "(c) 2013 Laurent Montel" ) );

    about->addAuthor( ki18n( "Laurent Montel" ), KLocalizedString(), "montel@kde.org" );
    KGlobal::locale()->insertCatalog(QLatin1String("libpimactivity"));
    setAboutData( about );

    initGUI();
}

KCMPimActivity::~KCMPimActivity()
{
}

void KCMPimActivity::initGUI()
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing( KDialog::spacingHint() );
    layout->setMargin( 0 );
    setLayout(layout);
    mManager = new PimActivity::ActivityManager(this);
    mConfigure = new PimActivity::ConfigureActivityWidget(mManager);
    connect(mConfigure, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
    layout->addWidget(mConfigure);
}

void KCMPimActivity::load()
{
    mConfigure->readConfig();
}

void KCMPimActivity::save()
{
    mConfigure->writeConfig();
}

void KCMPimActivity::defaults()
{
    mConfigure->defaults();
}

#include "kcmpimactivity.moc"
