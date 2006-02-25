/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "kornaccountcfgimpl.h"

#include "account_input.h"
#include "kio_proto.h"
#include "password.h"
#include "protocol.h"
#include "protocols.h"

#include <kconfigbase.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kurlrequester.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qptrvector.h>
#include <qlayout.h>
#include <qmap.h>
#include <qlabel.h>
#include <qwidget.h>

KornAccountCfgImpl::KornAccountCfgImpl( QWidget * parent, const char * name )
	: KornAccountCfg( parent, name ),
	_config( 0 ),
	_fields( 0 ),
	_urlfields( 0 ),
	_boxnr( 0 ),
	_accountnr( 0 ),
	_vlayout( 0 ),
	_protocolLayout( 0 ),
	_groupBoxes( 0 ),
	_accountinput( new QPtrList< AccountInput >() )
{
	connect( parent, SIGNAL( okClicked() ), this, SLOT( slotOK() ) );
	connect( parent, SIGNAL( cancelClicked() ), this, SLOT( slotCancel() ) );
	
	this->cbProtocol->insertStringList( Protocols::getProtocols() );

	_accountinput->setAutoDelete( true );

}
	
KornAccountCfgImpl::~KornAccountCfgImpl()
{
	delete _accountinput;
}

void KornAccountCfgImpl::readConfig( KConfigGroup *config, QMap< QString, QString > *entries, int boxnr, int accountnr )
{
	AccountInput *input;
	
	_config = config;

	_accountinput->clear();

	this->cbProtocol->setCurrentText( _config->readEntry( "protocol", "mbox" ) );
        slotProtocolChanged( this->cbProtocol->currentText() );
	const Protocol *protocol = Protocols::getProto( _config->readEntry( "protocol", "mbox" ) );

	protocol->readEntries( entries );

	(*entries)[ "password" ] = KOrnPassword::readKOrnPassword( boxnr, accountnr, *config );
	
	for( input = _accountinput->first(); input; input = _accountinput->next() )
		if( entries->contains( input->configName() ) )
			input->setValue( *(entries->find( input->configName() ) ) );
	
	this->edInterval->setText( _config->readEntry( "interval", "300" ) );
	
	this->chUseBox->setChecked( _config->readBoolEntry( "boxsettings", true ) );
	this->edRunCommand->setURL( _config->readEntry( "newcommand", "" ) );
	this->edPlaySound->setURL( _config->readEntry( "sound", "" ) );
	this->chPassivePopup->setChecked( _config->readBoolEntry( "passivepopup", false ) );
	this->chPassiveDate->setChecked( _config->readBoolEntry( "passivedate", false ) );

	_boxnr = boxnr;
	_accountnr = accountnr;
}

void KornAccountCfgImpl::writeConfig()
{
	AccountInput *input;
	const Protocol *protocol = Protocols::getProto( this->cbProtocol->currentText() );

	if( !protocol )
	{
		kdWarning() << "An error occured during writing the account information: protocol does not exist" << endl;
		return;
	}
	
	_config->writeEntry( "protocol", this->cbProtocol->currentText() );
		
	QMap< QString, QString > *map = new QMap< QString, QString >;
	QMap< QString, QString >::ConstIterator it;
	for( input = _accountinput->first(); input; input = _accountinput->next() )
		map->insert( input->configName(), input->value() );

	protocol->writeEntries( map );

	if( map->contains( "password" ) )
	{
		KOrnPassword::writeKOrnPassword( _boxnr, _accountnr, *_config, *map->find( "password" ) );
		map->erase( "password" );
	}

	for( it = map->begin(); it != map->end(); ++it )
		_config->writeEntry( it.key(), it.data() );

	delete map;
	
	_config->writeEntry( "interval", this->edInterval->text().toInt() );

	_config->writeEntry( "boxsettings", this->chUseBox->isChecked() );
	_config->writeEntry( "newcommand", this->edRunCommand->url() );
	_config->writeEntry( "sound", this->edPlaySound->url() );
	_config->writeEntry( "passivepopup", this->chPassivePopup->isChecked() );
	_config->writeEntry( "passivedate", this->chPassiveDate->isChecked() );
}

void KornAccountCfgImpl::slotSSLChanged()
{
	AccountInput *input;
	const Protocol* protocol = Protocols::getProto( this->cbProtocol->currentText() );
	bool ssl = false;
	
	if( !protocol )
		return;

	for( input = _accountinput->first(); input; input = _accountinput->next() )
		if( ( input->configName() == "ssl" && input->value() == "true" ) || input->value() == "ssl" )
			ssl = true;

	for( input = _accountinput->first(); input; input = _accountinput->next() )
		if( input->configName() == "port" && ( input->value() == QString::number( protocol->defaultPort( !ssl ) ) ) )
			input->setValue( QString::number( protocol->defaultPort( ssl ) ) );
}
	
void KornAccountCfgImpl::slotOK()
{
	writeConfig();
}

void KornAccountCfgImpl::slotCancel()
{
}

void KornAccountCfgImpl::slotProtocolChanged( const QString& proto )
{
	const Protocol *protocol = Protocols::getProto( proto );
	QStringList *groupBoxes = new QStringList;
	int counter = 1;

	protocol->configFillGroupBoxes( groupBoxes );
	
	_accountinput->clear();
	delete _groupBoxes;
	delete _protocolLayout;
	delete _vlayout;
	_vlayout = new QVBoxLayout( this->server_tab, groupBoxes->count() + 1 );
	_vlayout->setSpacing( 10 );
	_vlayout->setMargin( 10 );

	_protocolLayout = new QHBoxLayout( _vlayout );
	_protocolLayout->addWidget( this->lbProtocol );
	_protocolLayout->addWidget( this->cbProtocol );

	QStringList::iterator it;
	counter = 0;
	_groupBoxes = new QPtrVector< QWidget >( groupBoxes->count() );
	_groupBoxes->setAutoDelete( true );
	for( it = groupBoxes->begin(); it != groupBoxes->end(); ++it )
	{
		_groupBoxes->insert( counter, new QGroupBox( (*it), this->server_tab, "groupbox" ) );
		_vlayout->addWidget( _groupBoxes->at( counter ) );
		++counter;
	}
	delete groupBoxes;

	AccountInput *input;
	protocol->configFields( _groupBoxes, this, _accountinput );
	
	for( unsigned int groupCounter = 0; groupCounter < _groupBoxes->count(); ++groupCounter )
	{
		int counter = 0;
		QGridLayout *grid = new QGridLayout( _groupBoxes->at( groupCounter ), 0, 2 );
		grid->setSpacing( 10 );
		grid->setMargin( 15 );
		for( input = _accountinput->first(); input; input = _accountinput->next() )
		{
			if( input->leftWidget() && _groupBoxes->at( groupCounter ) == input->leftWidget()->parent() )
			{
				grid->addWidget( input->leftWidget(), counter, 0 );
				if( input->rightWidget() && _groupBoxes->at( groupCounter ) == input->rightWidget()->parent() )
					grid->addWidget( input->rightWidget(), counter, 1 );
				++counter;
			} else {
				if( input->rightWidget() && _groupBoxes->at( groupCounter ) == input->rightWidget()->parent() )
				{
					grid->addWidget( input->rightWidget(), counter, 1 );
					++counter;
				}
			}
		}

		_groupBoxes->at( groupCounter )->show();
	}

	this->lbInterval->setEnabled( proto != "process" && proto != "dcop" );
	this->edInterval->setEnabled( proto != "process" && proto != "dcop" );
	
	this->server_tab->updateGeometry();
}

#include "kornaccountcfgimpl.moc"
