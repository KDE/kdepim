/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#include "archivemaildialog.h"

#include "archivemailwidget.h"
#include "kdepim-version.h"

#include <mailcommon/util/mailutil.h>

#include <KGlobal>
#include <KLocale>
#include <KMenu>
#include <KHelpMenu>
#include <KAboutData>

#include <QHBoxLayout>

namespace {
inline QString archiveMailCollectionPattern() { return  QLatin1String( "ArchiveMailCollection \\d+" ); }
}

ArchiveMailDialog::ArchiveMailDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Configure Archive Mail Agent" ) );
    setWindowIcon( KIcon( QLatin1String("kmail") ) );
    setButtons( Help | Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );
    QWidget *mainWidget = new QWidget( this );
    QHBoxLayout *mainLayout = new QHBoxLayout( mainWidget );
    mainLayout->setSpacing( KDialog::spacingHint() );
    mainLayout->setMargin( KDialog::marginHint() );
    mWidget = new ArchiveMailWidget(this);
    mWidget->setObjectName(QLatin1String("archivemailwidget"));
    connect(mWidget, SIGNAL(archiveNow(ArchiveMailInfo*)), this, SIGNAL(archiveNow(ArchiveMailInfo*)));
    mainLayout->addWidget(mWidget);
    setMainWidget( mainWidget );
    connect(this, SIGNAL(okClicked()), SLOT(slotSave()));
    readConfig();

    mAboutData = new KAboutData(
                QByteArray( "archivemailagent" ),
                QByteArray(),
                ki18n( "Archive Mail Agent" ),
                QByteArray( KDEPIM_VERSION ),
                ki18n( "Archive emails automatically." ),
                KAboutData::License_GPL_V2,
                ki18n( "Copyright (C) 2012, 2013, 2014 Laurent Montel" ) );

    mAboutData->addAuthor( ki18n( "Laurent Montel" ),
                         ki18n( "Maintainer" ), "montel@kde.org" );

    mAboutData->setProgramIconName( QLatin1String("kmail") );
    mAboutData->setTranslator( ki18nc( "NAME OF TRANSLATORS", "Your names" ),
                             ki18nc( "EMAIL OF TRANSLATORS", "Your emails" ) );


    KHelpMenu *helpMenu = new KHelpMenu(this, mAboutData, true);
    //Initialize menu
    KMenu *menu = helpMenu->menu();
    helpMenu->action(KHelpMenu::menuAboutApp)->setIcon(KIcon(QLatin1String("kmail")));
    setButtonMenu( Help, menu );
}

ArchiveMailDialog::~ArchiveMailDialog()
{
    writeConfig();
    delete mAboutData;
}

void ArchiveMailDialog::slotNeedReloadConfig()
{
    mWidget->needReloadConfig();
}

static const char myConfigGroupName[] = "ArchiveMailDialog";

void ArchiveMailDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), myConfigGroupName );

    const QSize size = group.readEntry( "Size", QSize(500, 300) );
    if ( size.isValid() ) {
        resize( size );
    }

    mWidget->restoreTreeWidgetHeader(group.readEntry("HeaderState",QByteArray()));
}

void ArchiveMailDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), myConfigGroupName );
    group.writeEntry( "Size", size() );
    mWidget->saveTreeWidgetHeader(group);
    group.sync();
}

void ArchiveMailDialog::slotSave()
{
    mWidget->save();
}


