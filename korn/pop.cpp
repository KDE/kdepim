/*
* pop.cpp -- Implementation of class KPop3Drop.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Tue Apr 21 18:12:41 EST 1998
*/

#include<stdio.h>

#include<kconfigbase.h>
#include <kdebug.h>
#include<kmdcodec.h>
#include <klocale.h>
#include<kmessagebox.h>

#include<mimelib/pop.h>

#include"utils.h"
#include"pop.h"
#include"popcfg.h"
#include"dropdlg.h"
#include "intid.h"
#include "mailsubject.h"


#include <mimelib/message.h>
#include <mimelib/headers.h>
#include <mimelib/text.h>
#include <mimelib/mboxlist.h>
#include <kmime_util.h>

KPop3Drop::KPop3Drop()
  : KPollableDrop(),
    _port( DefaultPort ),
    _user(""),
    _password(""),
    _savePassword( false ),
    _valid (false ),
		_apopAuth ( false ),
    _pop( 0 )
{
}

QValueVector<KornMailSubject> * KPop3Drop::doReadSubjects(bool * stop)
{
	QValueVector<KornMailSubject> * result = new QValueVector<KornMailSubject>();

	// open connection
	if (!openConnection())
		return result;

	// load the list of mail ids from the mail box
	int ret = _pop->List();

	if( ( ret == '-' ) || ( !ret ) )
	{
		_pop->Quit();
		_pop->Close();

		return result;
	}

	// parse the list, count mails
	DwString response = _pop->MultiLineResponse();
	int totalSubjects = -1, pos = 0;
	while (pos >= 0)
	{
		++totalSubjects;
		pos = response.find('\n', pos+1);
	}

	// set the progress bar
	if (totalSubjects)
		emit readSubjectsTotalSteps(totalSubjects);
	int subjectNum = 0;
	emit readSubjectsProgress(subjectNum);

	// loop through all mials
	while (response.length() && (!stop || !*stop))
	{
		// one line in the List() response (contains mail id)
		DwString line = response;
		int pos = response.find('\n');
		if (pos != DwString::npos)
		{
			line = response.substr(0, pos);

			// cut current line from resonse string
			response = response.substr(pos+1);
		}
		else
			response = "";

		// convert id
		int id=0, octets=0;
		sscanf( line.c_str(), "%d %d", &id, &octets );

		// prepare KornMailSubject instance
		KornMailSubject mailSubject = KornMailSubject(new KornIntId(id));
		mailSubject.setSize(octets);

		// read mail header
		ret = _pop->Top(id, 0);
		if( ( ret != '-' ) && ret )
		{
			// parse result
			DwMessage mMsg;
			DwString messageHeader = _pop->MultiLineResponse();
			mMsg.FromString(messageHeader);
			mMsg.Parse();

			// extract sender, subject and date
			DwHeaders & header = mMsg.Headers();
			mailSubject.setSender(header.FieldBody("From").AsString().c_str());
			QCString subject = header.FieldBody("Subject").AsString().c_str();
			const char *usedCS = NULL;

			// decode subject if encoded in some way
			mailSubject.setSubject(KMime::decodeRFC2047String(subject, &usedCS, "base64", false));
			mailSubject.setDate(header.Date().AsUnixTime());
			mailSubject.setHeader(messageHeader.c_str(), false);
		}

		// store subject in result vector
		result->push_back(mailSubject);

		// set progress bar
		emit readSubjectsProgress(++subjectNum);
	}

	// close connection
	_pop->Quit();
	_pop->Close();

	return result;
}

bool KPop3Drop::deleteMails(QPtrList<const KornMailId> * ids, bool * stop)
{
	// set progress bar
	int count = ids->count();
	if (count)
		emit deleteMailsTotalSteps(count);
	count = 0;
	emit deleteMailsProgress(count);

	// open connection
	if (!openConnection())
		return false;

	// loop through all ids
	for ( const KornMailId * item = ids->first(); item  && (!stop || !*stop); item = ids->next() )
	{
		// delete mail
		int ret = _pop->Dele(((const KornIntId *)item)->getId());
		if( ( ret == '-' ) || ( !ret ) )
		{
			// terminate on error. The reload of mail subjects
			// will show the not deleted mails.
			_pop->Quit();
			_pop->Close();

			return true;
		}

		// set progress bar
		emit deleteMailsProgress(++count);
	}

	// close connection
	_pop->Quit();
	_pop->Close();
	return true;
}

QString KPop3Drop::readMail(const KornMailId * id, bool * /*stop*/)
{
	// open connection
	if (!openConnection())
		return "";

	// read mail
	int ret = _pop->Retr(((KornIntId *)id)->getId());
	if( ( ret == '-' ) || ( !ret ) )
	{
		// return "" on error
		_pop->Quit();
		_pop->Close();

		return "";
	}

	//return result;
	DwString result = _pop->MultiLineResponse();

	_pop->Quit();
	_pop->Close();

	return result.c_str();
}

void KPop3Drop::setPopServer(const QString & server, int port, bool apop)
{
	_server = server;
	_port	= port;
	_apopAuth = apop;
}

void KPop3Drop::setUser(const QString & user, const QString & password,
	bool savepass )
{
	_user = user;
	_password = password;
	_savePassword = savepass;

	_valid = true;
}

bool KPop3Drop::openConnection()
{
	if( _pop == 0 ) {
		_pop = new DwPopClient;
	}

	//kdDebug() << "POP3: server = " << _server << endl;
	int ret = _pop->Open(_server.ascii(), _port );

	if( !ret ) {
		_valid = false;
		if( _pop->IsOpen() ) _pop->Close();
		return false;
	}

	DwString response;

	if( _apopAuth ){
		response = _pop->SingleLineResponse();
		DwString apopTimestamp;

		int endMark = response.rfind( '>' );
		int startMark = response.rfind( '<' );

		//cout << "endMark " << endMark << " startMark " << startMark << endl;

		//Check if this server supports APOP.
		if ( endMark == -1 || startMark == -1 ){
			//KMessageBox::error( 0, _server + i18n(" does not support APOP authentication.") );
			_valid = false;
			_pop->Quit();
			_pop->Close();

			return false;
		}

		//Get the timestamp, e.g. <25799.1017300043@mail.host.com>
		//*including* angle brackets
		apopTimestamp = response.substr( startMark, endMark - startMark +1 );
		//cout << "apopTimestamp: " << apopTimestamp << endl;

		apopTimestamp.append( _password.ascii() );
		KMD5 context( apopTimestamp.c_str() );
		//cout <<  "pass " << _password.ascii() << "digest \""
		//	<< context.hexDigest().data() << "\"" << endl;

		ret = _pop->Apop( _user.ascii(), context.hexDigest().data() );

		if( ( ret == '-' ) || ( !ret ) ){
			_valid = false;
			_pop->Quit();
			_pop->Close();

			return false;
		}

	} else {
		//kdDebug() << "POP3: user = " << _user << endl;
		ret = _pop->User(_user.ascii());

		if( ( ret == '-' ) || ( !ret ) ){
			_valid = false;
			_pop->Quit();
			_pop->Close();

		return false;
		}

		//kdDebug() << "POP3: password = " << _password << endl;
		ret = _pop->Pass(_password.ascii());

		if( ( ret == '-' ) || ( !ret ) ){
			_valid = false;
			_pop->Quit();
			_pop->Close();

		return false;
		}
	return true;
	}
        return false; // Shut up spurious compiler warning.
}

void KPop3Drop::recheck()
{
	if (!openConnection())
		return;
	int ret = _pop->Stat();

	if( ( ret == '-' ) || ( !ret ) ){
		_valid = false;
		_pop->Quit();
		_pop->Close();

		return;
	}

	DwString response = _pop->SingleLineResponse();

	int newcount=0, octets=0;

	sscanf( response.c_str(), "+OK %d %d", &newcount, &octets );

	_pop->Quit();
	_pop->Close();

	if( newcount != count() ) {
		emit changed( newcount );
	}

	_valid = true;
	return;
}

bool KPop3Drop::valid()
{
	return _valid;
}

bool KPop3Drop::apopAuth()
{
	return _apopAuth;
}

KPop3Drop::~KPop3Drop()
{
	delete _pop;
}

KMailDrop* KPop3Drop::clone() const
{
	KPop3Drop *clone = new KPop3Drop;

	*clone = *this;

	return clone;
}

bool KPop3Drop::readConfigGroup( const KConfigBase& cfg )
{
	QString val;
	KPollableDrop::readConfigGroup( cfg );

	val = cfg.readEntry(fu(HostConfigKey));
	if( val.isEmpty() ) { _valid = false; return false; }
	setPopServer( val, cfg.readNumEntry(fu(PortConfigKey), DefaultPort ),
			cfg.readBoolEntry(fu(ApopConfigKey), false) );

	_user = cfg.readEntry(fu(UserConfigKey));
	if( _user.isEmpty() ) { _valid = false; return false; }

	_password = cfg.readEntry(fu(PassConfigKey));

	if( _password.isEmpty() ) {
		_savePassword = false;
	}
	else {
		_savePassword = true;
		decrypt( _password );
	}

	return true;
}

bool KPop3Drop::writeConfigGroup( KConfigBase& cfg ) const
{
	KPollableDrop::writeConfigGroup( cfg );
	QString p;

	if( _savePassword == true ) {
		p = _password;
		encrypt( p );
	}

	cfg.writeEntry(fu(HostConfigKey), _server );
	cfg.writeEntry(fu(PortConfigKey), _port );
	cfg.writeEntry(fu(ApopConfigKey), _apopAuth );
	cfg.writeEntry(fu(UserConfigKey), _user );
	cfg.writeEntry(fu(PassConfigKey), p );

	return true;
}

KPop3Drop& KPop3Drop::operator = ( const KPop3Drop& other )
{
	setPopServer( other._server, other._port, other._apopAuth );
	setUser( other._user,other._password );
	setFreq( other.freq() );

	return *this;
}

void KPop3Drop::addConfigPage( KDropCfgDialog *dlg )
{
	dlg->addConfigPage( new KPopCfg( this ) );

	KPollableDrop::addConfigPage( dlg );
}

void KPop3Drop::encrypt( QString& str )
{
	unsigned int i, val;
	unsigned int len = str.length();
	QString result;

	for ( i=0; i < len; i++ )
	{
		char c = str[ i ].latin1();

		val = c - ' ';
		val = (255-' ') - val;
		result[i] = (char)(val + ' ');
	}
	result[i] = '\0';
}

void KPop3Drop::decrypt( QString& str )
{
	encrypt( str );
}

const char *KPop3Drop::HostConfigKey = "host";
const char *KPop3Drop::PortConfigKey = "port";
const char *KPop3Drop::ApopConfigKey = "apop";
const char *KPop3Drop::UserConfigKey = "user";
const char *KPop3Drop::PassConfigKey = "pass";
const char *KPop3Drop::SavePassConfigKey = "savepass";
const int  KPop3Drop::DefaultPort	= 110;
