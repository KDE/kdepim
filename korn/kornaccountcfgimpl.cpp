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
#include "settings.h"

#include <kconfigbase.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kurlrequester.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLayout>
#include <QMap>
#include <QLabel>
#include <QList>
#include <QWidget>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

KornAccountCfgImpl::KornAccountCfgImpl( QWidget * parent, Settings *glob_settings, AccountSettings *settings )
	: QWidget( parent ),
	Ui_KornAccountCfg(),
	m_glob_settings( glob_settings ),
	m_settings( settings ),
	m_fields( 0 ),
	m_urlfields( 0 ),
	m_boxnr( 0 ),
	m_accountnr( 0 ),
	m_vlayout( 0 ),
	m_protocolLayout( 0 ),
	m_groupBoxes( 0 ),
	m_accountinput( new QList< AccountInput* >() )
{
	setupUi( this );

	connect( parent, SIGNAL( okClicked() ), this, SLOT( slotOK() ) );
	connect( parent, SIGNAL( cancelClicked() ), this, SLOT( slotCancel() ) );
	connect( chUseBox, SIGNAL(toggled(bool)), gbNewMail, SLOT(setDisabled(bool)) );
	connect( chPassivePopup, SIGNAL(toggled(bool)), chPassiveDate, SLOT(setEnabled(bool)) );
	connect( cbProtocol, SIGNAL(activated(const QString&)), this, SLOT(slotProtocolChanged(const QString&)) );

	this->cbProtocol->insertItems( this->cbProtocol->count(), Protocols::getProtocols() );
}

KornAccountCfgImpl::~KornAccountCfgImpl()
{
	while( !m_accountinput->isEmpty() )
		delete m_accountinput->takeFirst();
	delete m_accountinput;
}

void KornAccountCfgImpl::readConfig()
{
	QMap< QString, QString > *entries = new QMap< QString, QString >( m_settings->readEntries() );

	this->edName->setText( m_settings->accountName() );

	while( !m_accountinput->isEmpty() )
		delete m_accountinput->takeFirst();

	this->cbProtocol->setCurrentIndex( this->cbProtocol->findText( m_settings->protocol() ) );
        slotProtocolChanged( this->cbProtocol->currentText() );
	const Protocol *protocol = Protocols::getProto( m_settings->protocol() );

	protocol->readEntries( entries );

	for( int xx = 0; xx < m_accountinput->size(); ++xx )
		if( entries->contains( m_accountinput->at( xx )->configName() ) )
			m_accountinput->at( xx )->setValue( *(entries->find( m_accountinput->at( xx )->configName() ) ) );

	this->edInterval->setText( QString::number( m_settings->interval() ) );

	this->chUseBox->setChecked( m_settings->useBoxSettings() );
	this->edRunCommand->setUrl( m_settings->command() );
	this->edPlaySound->setUrl( m_settings->sound() );
	this->chPassivePopup->setChecked( m_settings->passivePopup() );
	this->chPassiveDate->setChecked( m_settings->passiveDate() );

	delete entries;
}

void KornAccountCfgImpl::writeConfig()
{
	const Protocol *protocol = Protocols::getProto( this->cbProtocol->currentText() );

	if( !protocol )
	{
		kWarning() << "An error occurred during writing the account information: protocol does not exist" << endl;
		return;
	}

	m_settings->setAccountName( this->edName->text() );
	m_settings->setProtocol( this->cbProtocol->currentText() );

	QMap< QString, QString > *map = new QMap< QString, QString >;
	QMap< QString, QString >::ConstIterator it;
	for( int xx = 0; xx < m_accountinput->size(); ++xx )
		map->insert( m_accountinput->at( xx )->configName(), m_accountinput->at( xx )->value() );

	protocol->writeEntries( map );

	m_settings->writeEntries( *map );
	
	//for( it = map->begin(); it != map->end(); ++it )
	//	m_config->writeEntry( it.key(), it.value() );

	delete map;

	m_settings->setInterval( this->edInterval->text().toInt() );

	m_settings->setUseBoxSettings( this->chUseBox->isChecked() );
	m_settings->setCommand( this->edRunCommand->url().url() );
	m_settings->setSound( this->edPlaySound->url().url() );
	m_settings->setPassivePopup( this->chPassivePopup->isChecked() );
	m_settings->setPassiveDate( this->chPassivePopup->isChecked() );
}

void KornAccountCfgImpl::slotSSLChanged()
{
	const Protocol* protocol = Protocols::getProto( this->cbProtocol->currentText() );
	bool ssl = false;

	if( !protocol )
		return;

	for( int xx = 0; xx < m_accountinput->size(); ++xx )
		if( ( m_accountinput->at( xx )->configName() == "ssl" && m_accountinput->at( xx )->value() == "true" ) ||
		    m_accountinput->at( xx )->value() == "ssl" )
			ssl = true;

	for( int xx = 0; xx < m_accountinput->size(); ++xx )
		if( m_accountinput->at( xx )->configName() == "port" &&
		    ( m_accountinput->at( xx )->value() == QString::number( protocol->defaultPort( !ssl ) ) ) )
			m_accountinput->at( xx )->setValue( QString::number( protocol->defaultPort( ssl ) ) );
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

	while( !m_accountinput->isEmpty() )
		delete m_accountinput->takeFirst();

	if( m_groupBoxes )
	{
		for( int xx = 0; xx < m_groupBoxes->size(); ++xx )
			delete m_groupBoxes->at( xx );
		delete m_groupBoxes;
	}
	delete m_protocolLayout;
	delete m_vlayout;
	m_vlayout = new QVBoxLayout( this->server_tab );
	m_vlayout->setSpacing( 10 );
	m_vlayout->setMargin( 10 );

	m_protocolLayout = new QHBoxLayout();
	m_vlayout->addLayout( m_protocolLayout );
	m_protocolLayout->addWidget( this->lbProtocol );
	m_protocolLayout->addWidget( this->cbProtocol );

	QStringList::iterator it;
	counter = 0;
	m_groupBoxes = new QVector< QWidget* >( groupBoxes->count() );
	for( it = groupBoxes->begin(); it != groupBoxes->end(); ++it )
	{
		(*m_groupBoxes)[ counter ] = new QGroupBox( (*it), this->server_tab );
		m_vlayout->addWidget( m_groupBoxes->at( counter ) );
		++counter;
	}
	delete groupBoxes;

	AccountInput *input;
	protocol->configFields( m_groupBoxes, this, m_accountinput );

	for( int groupCounter = 0; groupCounter < m_groupBoxes->count(); ++groupCounter )
	{
		int counter = 0;
		QGridLayout *grid = new QGridLayout( m_groupBoxes->at( groupCounter ) );
		grid->setSpacing( 10 );
		grid->setMargin( 15 );
		for( int xx = 0; xx < m_accountinput->size(); ++xx )
		{
			input = m_accountinput->at( xx );
			if( input->leftWidget() && m_groupBoxes->at( groupCounter ) == input->leftWidget()->parent() )
			{
				grid->addWidget( input->leftWidget(), counter, 0 );
				if( input->rightWidget() && m_groupBoxes->at( groupCounter ) == input->rightWidget()->parent() )
					grid->addWidget( input->rightWidget(), counter, 1 );
				++counter;
			} else {
				if( input->rightWidget() && m_groupBoxes->at( groupCounter ) == input->rightWidget()->parent() )
				{
					grid->addWidget( input->rightWidget(), counter, 1 );
					++counter;
				}
			}
		}

		m_groupBoxes->at( groupCounter )->show();
	}

	// Enable / disable interval input field
	this->lbInterval->setEnabled( protocol->isPollable() );
	this->edInterval->setEnabled( protocol->isPollable() );

	this->server_tab->updateGeometry();
}

#include "kornaccountcfgimpl.moc"
