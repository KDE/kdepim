
#include<assert.h>
#include<ctype.h>


#include<kconfig.h>

#include"utils.h"
#include"kornset.h"

KornSettings::KornSettings( KConfigBase *config )
	: _config( config ), 
	_layout( Horizontal ),
	_posSet( false ),
	_valid ( true )
{
//	_noMouse	= checkNoMouse();
}


bool KornSettings::readConfig()
{
	_config->setGroup(fu("Korn"));

// saved layout

	if ( _config->hasKey(fu("layout")) ){
		QString layoutStr = _config->readEntry(fu("layout"));

		layoutStr = layoutStr.stripWhiteSpace();

		QChar style = layoutStr[0].upper();

		if( style == 'V' ) {
			_layout = Vertical;
		}
		else if ( style == 'D' ) {
			_layout = Dock;
		}
		else {
			_layout = Horizontal;
		}
	}
	else {
		_layout = Dock;
	}

// Saved geometry

	_remember = _config->readNumEntry(fu("rememberPos"), 0);

	if( _remember && _config->hasKey(fu("posX")) ) {
		_geom.setX( _config->readNumEntry(fu("posX")) );
		_geom.setY( _config->readNumEntry(fu("posY")) );
		_posSet = TRUE;

	} else {
		_posY = _posX = 0;
	}

	return true;
}

bool KornSettings::writeConfig() const
{
	QString str;

	// save layout
	switch ( layout() ) {
		case Horizontal:
			str = fu("Horizontal");
			break;
		case Vertical:
			str = fu("Vertical");
			break;
		case Dock:
			str = fu("Docked");
			break;
	}
	_config->writeEntry(fu("layout"), str );

	return true;
}
