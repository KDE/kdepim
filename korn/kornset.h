
#ifndef SSK_KORNSET_H
#define SSK_KORNSET_H

class KConfigBase;

#include<qrect.h>

/**
*/
class KornSettings
{
public:
	enum Layout { Horizontal, Vertical, Dock };

private:
	KConfigBase *	_config;

	Layout	_layout;
	int 	_posX;
	int	_posY;
	bool	_remember;
	bool	_posSet;
	bool	_valid;
	QRect	_geom;

	bool	_noMouse;

	/** 
	  * Check if --nomouse is set.
	 */
	static bool checkNoMouse();
	static bool checkDock();


public:
	const KConfigBase * config() const { return _config; }

	Layout layout() const { return _layout; }
	void setLayout( Layout t ) { _layout = t; }

	int  posX() const { return _posX; }
	void setPosX( int  t ) { _posX = t; }

	int posY() const { return _posY; }
	void setPosY( int t ) { _posY = t; }

	bool remember() const { return _remember; }
	void setRemember( bool t ) { _remember = t; }

	bool posSet() const { return _posSet; }
	void setPosSet( bool t ) { _posSet = t; }

	bool valid() const { return _valid; }
	void setValid( bool t ) { _valid = t; }

	QRect geom() const { return _geom; }
	void setGeom( QRect t ) { _geom = t; }

	bool noMouse() const { return _noMouse; }
	void setNoMouse( bool t ) { _noMouse = t; }

public:
	KornSettings( KConfigBase *config );

public:
	bool readConfig();
	bool writeConfig() const;

};

#endif
