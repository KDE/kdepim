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
#include "picksmscenter.h"
#include "kmobiletoolshelper.h"

#include <qlayout.h>
#include <klocale.h>
#include <kconfig.h>
#include <k3listviewsearchline.h>
#include <k3listview.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

class PickSMSCenterPrivate {
    public:
        PickSMSCenterPrivate()
    : listview(NULL), config(NULL) {}

    K3ListView *listview;
    QString s_smsCenter;
    KConfig *config;
};


PickSMSCenter::PickSMSCenter(QWidget *parent)
    : KDialog(parent), d(new PickSMSCenterPrivate)
{
    setCaption( i18nc( "Pick SMS Center from list", "Choose Your Operator SMS Center") );
    setButtons( Ok | Cancel );
    enableButtonOk (false);
    QVBoxLayout *lay=new QVBoxLayout(mainWidget());
    K3ListViewSearchLineWidget *slwidget=new K3ListViewSearchLineWidget( 0, this);
    lay->addWidget(slwidget);
    d->listview=new K3ListView(this);
    lay->addWidget(d->listview);
    d->listview->addColumn( i18nc("Network name for SMS Center", "Network Name") );
    d->listview->addColumn( i18nc("SMS Center number", "Number") );
    connect(d->listview, SIGNAL(clicked( Q3ListViewItem* )), this, SLOT(clicked( Q3ListViewItem* ) ) );
    connect(d->listview, SIGNAL(doubleClicked( Q3ListViewItem*, const QPoint&, int ) ), this, SLOT(doubleClicked( Q3ListViewItem*, const QPoint&, int )) );
    slwidget->createSearchLine(d->listview);
    resize(400,500);
    initList();
}


PickSMSCenter::~PickSMSCenter()
{
    delete d;
}


#include "picksmscenter.moc"


/*!
    \fn PickSMSCenter::initList()
 */
void PickSMSCenter::initList()
{
    QString file=KGlobal::dirs()->findResource( "data", "kmobiletools/operatorsdata" );
    if(file.isNull() )
    {
        KMessageBox::error( this, i18n("Operators data not found.") );
        return;
    }

    d->config=new KConfig( file, KConfig::NoGlobals );
    QStringList operators=d->config->groupList();
    for(QStringList::Iterator it=operators.begin(); it!=operators.end(); ++it)
    {
        const KConfigGroup cg( d->config, *it );
        if(cg.readEntry( "smscenter").isNull() ) continue;
        new K3ListViewItem(d->listview, *it, cg.readEntry( "smscenter") );
    }
    delete d->config;
}


/*!
    \fn PickSMSCenter::clicked ( QListViewItem * item )
 */
void PickSMSCenter::clicked ( Q3ListViewItem * item )
{
    if(!item )
    {
        enableButtonOk(false);
        d->s_smsCenter.clear();
        return;
    }
    enableButtonOk( true );
    d->s_smsCenter=item->text(1);
}


/*!
    \fn PickSMSCenter::doubleClicked( QListViewItem *, const QPoint &, int )
 */
void PickSMSCenter::doubleClicked( Q3ListViewItem *item, const QPoint &, int )
{
    clicked(item);
    if(!item)return;
    done(Accepted);
}

const QString PickSMSCenter::smsCenter() { return d->s_smsCenter;}


/*!
    \fn PickSMSCenter::smsCenterName(const QString &smsCenter)
 */
QString PickSMSCenter::smsCenterName(const QString &smsCenter)
{
    QString file=KGlobal::dirs()->findResource( "data", "kmobiletools/operatorsdata" );
//     if(file.isNull() ) return smsCenter;

    KConfig *config=new KConfig( file, KConfig::NoGlobals );
    QStringList operators=config->groupList();
    for(QStringList::Iterator it=operators.begin(); it!=operators.end(); ++it)
    {
        const KConfigGroup cg( config, *it );
        if( KMobileTools::KMobiletoolsHelper::compareNumbers( cg.readEntry( "smscenter"), smsCenter ) )
        {
            delete config;
            return *it;
        }
    }
    delete config;
    return smsCenter;
}
