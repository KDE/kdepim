/*
* ecfg.cpp -- Implementation of class KExternCfg.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Sat Nov 21 18:50:13 EST 1998
*/

#include <assert.h>
#include <qlineedit.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>

//#include"typolayout.h"

#include"ecfg.h"
#include"edrop.h"

KExternCfg::KExternCfg( KExternDrop *drop )
	: KMonitorCfg( drop ),
	_cmdEdit( 0 )
{
}

QString KExternCfg::name() const
{
	return i18n( "&Process" );
}

QWidget *KExternCfg::makeWidget( QWidget *parent )
{

	KExternDrop *d = dynamic_cast<KExternDrop *>(drop());
  assert(0 != d);

	QWidget *dlg = new QWidget( parent );
	QBoxLayout *l = new QVBoxLayout ( dlg, 10 );
	l->addSpacing(10);

	QGroupBox *aGroup = new QGroupBox( i18n( "Process" ), dlg );
	l->addWidget(aGroup);

	QGridLayout *slay = new QGridLayout( aGroup, 2, 2, 10 );
	slay->addRowSpacing( 0, 10 );

	slay->addWidget( new QLabel( i18n( "Shell command:" ), aGroup ), 1, 0 );

	_cmdEdit = new QLineEdit(aGroup);
	slay->addWidget( _cmdEdit, 1, 1 );

	if( d->valid() ) {
		_cmdEdit->setText( d->command() );
	}

	return dlg;
}

void KExternCfg::updateConfig()
{
	assert( _cmdEdit != 0 );

	KExternDrop *d = dynamic_cast<KExternDrop *>(drop());
  assert(0 != d);
	d->setCommand( _cmdEdit->text() );
}
#include "ecfg.moc"
