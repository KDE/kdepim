/*
* unixcfg.cpp -- Implementation of class KUnixCfg.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Mon Aug  3 02:08:36 EST 1998
*/

#include <assert.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>

#include "unixdrop.h"
#include "unixcfg.h"
//#include "typolayout.h"
#include "kfbrowsebtn.h"

KUnixCfg::KUnixCfg( KUnixDrop *drop ) 
	: KMonitorCfg( drop ),
	_fileEdit( 0 ) 
{
}

QString KUnixCfg::name() const
{
	return i18n( "&Mailbox" );
}

QWidget *KUnixCfg::makeWidget( QWidget *parent )
{
	KUnixDrop *d = dynamic_cast<KUnixDrop *>(drop());
  assert(0 != d);

	// layout

	QWidget *dlg = new QWidget( parent );
	QGridLayout *layout = new QGridLayout( dlg, 4, 2, 10 );
	layout->addRowSpacing(0, 10);
	layout->addRowSpacing(3, 10);

	// edit
	layout->addWidget(new QLabel( i18n( "Mbox path:" ), dlg ), 1, 0);

	_fileEdit = new KURLRequester( d->file(), dlg );
	layout->addWidget( _fileEdit, 1, 1);
       
	return dlg;
}

void KUnixCfg::updateConfig()
{
	assert( _fileEdit != 0 );
	KUnixDrop *d = dynamic_cast<KUnixDrop *>(drop());
  assert(0 != d);
	
	d->setFile( _fileEdit->lineEdit()->text() );
}

