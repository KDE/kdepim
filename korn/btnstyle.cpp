/*
* btnstyle.cpp -- Implementation of class KornBtnStyle.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Sat Apr 21 04:45:02 EST 2001
*/

#include"btnstyle.h"
#include"kornbutt.h"
#include"maildrop.h"
#include"kiconloader.h"

//
// Class: KornBtnIconStyle
//

void KornBtnIconStyle::update( int , bool newMessages )
{
	QString icon;

	if( newMessages )
		icon = _box->newIcon();
	else
		icon = _box->icon();

	QPixmap pix = KGlobal::iconLoader()->loadIcon( icon, KIcon::Desktop );
	_btn->setPixmap( pix );
}


//
// Class: KornBtnPlainTextStyle
//
void KornBtnPlainTextStyle::enable()
{
	_btn->setPixmap( NULL );
	_btn->setText(QString::number(_box->count()));
	
}

void KornBtnPlainTextStyle::disable()
{
}

void KornBtnPlainTextStyle::update( int msgcount, bool newMessages )
{
	QPalette p = _btn->palette();

	if (newMessages) {

		p.setColor(QColorGroup::Button, _box->newBgColour());
		p.setColor(QColorGroup::ButtonText, _box->newFgColour());
		p.setColor(QColorGroup::Foreground, _box->newFgColour());

	} else {

		p.setColor(QColorGroup::Button, _box->bgColour());
		p.setColor(QColorGroup::ButtonText, _box->fgColour());
		p.setColor(QColorGroup::Foreground, _box->fgColour());
	}

	_btn->setPalette(p);

	_btn->setText( QString::number( msgcount ) );
}


//
// Class: KornBtnColourTextStyle
//
void KornBtnColourTextStyle::enable()
{
	_btn->setPixmap( NULL );
	_btn->setText( QString::number( _box->count()) );
}

void KornBtnColourTextStyle::disable()
{
}

void KornBtnColourTextStyle::update( int msgcount, bool newMessages )
{
	QPalette p = _btn->palette();

	if (newMessages) {

		p.setColor(QColorGroup::Button, _box->newBgColour());
		p.setColor(QColorGroup::ButtonText, _box->newFgColour());
		p.setColor(QColorGroup::Foreground, _box->newFgColour());

	} else {

		p.setColor(QColorGroup::Button, _box->bgColour());
		p.setColor(QColorGroup::ButtonText, _box->fgColour());
		p.setColor(QColorGroup::Foreground, _box->fgColour());
	}

	_btn->setPalette(p);

	_btn->setText( QString::number( msgcount ) );
}

