/*
* newscfg.cpp -- Implementation of class KNewsCfg.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Mon Aug  3 15:41:58 EST 1998
*/

#include <assert.h>
#include <klocale.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>

//#include "typolayout.h"

#include "newscfg.h"
#include "news.h"

KNewsCfg::KNewsCfg( KNewsDrop *drop )
	: KMonitorCfg( drop ),
	_serverEdit( 0 ),
	_groupEdit ( 0 )
{
}

QString KNewsCfg::name() const
{
	return i18n( "&News" );
}

QWidget *KNewsCfg::makeWidget( QWidget *parent )
{
	KNewsDrop *d  = dynamic_cast<KNewsDrop *>(drop());
  assert(0 != d);

	QWidget *dlg = new QWidget ( parent );
	QGridLayout *layout = new QGridLayout( dlg, 3, 2, 10 );
	layout->addRowSpacing(0,10);

	QLabel *aLabel = new QLabel( i18n("Server:"), dlg );
	layout->addWidget(aLabel, 1, 0);

	_serverEdit = new QLineEdit( d->server(), dlg);
	layout->addWidget(_serverEdit, 1, 1);

	aLabel = new QLabel( i18n("Group:"), dlg );
	layout->addWidget(aLabel, 2, 0);

	_groupEdit = new QLineEdit( d->group(), dlg );
	layout->addWidget(_groupEdit, 2, 1);

	return dlg;
}

void KNewsCfg::updateConfig()
{
	assert( _serverEdit );
	assert( _groupEdit );

	KNewsDrop *d  = dynamic_cast<KNewsDrop *>(drop());
  assert(0 != d);

	d->setServer( _serverEdit->text() );
	d->setGroup( _groupEdit->text() );
}

