/*
* kiocfg.h -- Declaration of class KKIOCfg.
*/
#ifndef KEG_KIOCFG_H
#define KEG_KIOCFG_H

#include<qobject.h>

#include "moncfg.h"

class QLineEdit;
class KKioDrop;
class QLabel;
class QCheckBox;
class QComboBox;
template<class> class QPtrList;
class KIO_Protocol;

/**
* Configuration manager for @ref KImap4Drop monitors.
* @author Kurt Granroth (granroth@kde.org)
* @version $Id$
* Copyed and edited for kio by Mart Kelder
*/
class KKioCfg : public KMonitorCfg
{
Q_OBJECT
public:
	/**
	* KImapCfg Constructor
	*/
	KKioCfg( KKioDrop *drop );

	/**
	* KImapCfg Destructor
	*/
	virtual ~KKioCfg();
	
	virtual QString name() const;
	virtual QWidget *makeWidget( QWidget *parent );
	virtual void updateConfig();

	void addProtocol( KIO_Protocol * proto );	

private:
	KKioCfg& operator=( KKioCfg& );
	KKioCfg( const KKioCfg& );
	bool setComboItem( const QString & item );

	QPtrList<KIO_Protocol> *_protocols;

	QLabel *_serverLabel;
	QLabel *_portLabel;
	QLabel *_mailboxLabel;
	QLabel *_userLabel;
	QLabel *_pwdLabel;
	QLabel *_authLabel;
	
	QComboBox *_protoCombo;
	QLineEdit *_serverEdit;
	QLineEdit *_portEdit;
	QLineEdit *_mailboxEdit;
	QLineEdit *_userEdit;
	QLineEdit *_pwdEdit;
	QCheckBox *_savePass;
	QComboBox *_authCombo;

	KIO_Protocol *_this_protocol;
private slots:
	void protoChange( int );
};

#endif // KEG_KIOCFG_H
