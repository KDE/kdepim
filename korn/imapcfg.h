/*
* imapcfg.h -- Declaration of class KImapCfg.
*/
#ifndef KEG_IMAPCFG_H
#define KEG_IMAPCFG_H

#include "moncfg.h"

class QLineEdit;
class KImap4Drop;
class QCheckBox;

/**
* Configuration manager for @ref KImap4Drop monitors.
* @author Kurt Granroth (granroth@kde.org)
* @version $Id$
*/
class KImapCfg : public KMonitorCfg
{
public:
	/**
	* KImapCfg Constructor
	*/
	KImapCfg( KImap4Drop *drop );

	/**
	* KImapCfg Destructor
	*/
	virtual ~KImapCfg() {}
	
	virtual QString name() const;
	virtual QWidget *makeWidget( QWidget *parent );
	virtual void updateConfig();

private:
	KImapCfg& operator=( KImapCfg& );
	KImapCfg( const KImapCfg& );

	QLineEdit *_serverEdit;
	QLineEdit *_portEdit;
	QLineEdit *_mailboxEdit;
	QLineEdit *_userEdit;
	QLineEdit *_pwdEdit;
	QCheckBox *_savePass;
};

#endif // KEG_IMAPCFG_H
