/*  This file is part of the KDE mobile library.
    Copyright (C) 2003 Helge Deller <deller@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqlabel.h>
#include <tqwidget.h>
#include <tqcombobox.h>

#include "gnokiiconfigui.h"
#include "gnokiiconfig.h"

#define MODELS	"AT 1611 2110i 2148i 2711 2731 3110 3210 3310 3330 3360 3410 3810 " \
		"5100 5110 5120 5130 5160 5190 540 550 6110 6120 6130 6150 616x " \
		"6185 6190 6210 6230 6250 6310 6310i 6360 640 650 6510 6610 7110 7190 " \
		"7650 8110 8210 8250 8290 8310 8810 9110 9210 RPM-1"

#define CONNECTIONS "serial infrared irda dau9p dlr3p m2bus bluetooth tekram tcp"

#if defined(__linux__) || defined(__linux)
#define AVAILABLE_PORTS	"/dev/ttyS0 /dev/ttyS1 /dev/ttyS2 /dev/ttyS3 " \
			"/dev/ircomm0 /dev/ircomm1 /dev/irda /dev/rfcomm0 /dev/rfcomm1"
#else
#define AVAILABLE_PORTS	"/dev/ttyS0"
#endif 

#define BAUDRATES	"57600 38400 19200 14400 9600 4800 2400"


GnokiiConfig::GnokiiConfig( TQWidget* parent, const char* name, bool modal, WFlags fl )
	: GnokiiConfigUI(parent, name, modal, fl)
{
   TQStringList list = TQStringList::split(" ", MODELS);
   cb_Model->insertStringList(list);

   list = TQStringList::split(" ", CONNECTIONS);
   cb_Connection->insertStringList(list);

   list = TQStringList::split(" ", AVAILABLE_PORTS);
   cb_Port->insertStringList(list);

   list = TQStringList::split(" ", BAUDRATES);
   cb_Baud->insertStringList(list);
}

GnokiiConfig::~GnokiiConfig()
{
}

void GnokiiConfig::setValues(const TQString &model, const TQString &connection, const TQString &port, const TQString &baud)
{
   cb_Model->setCurrentText(model);
   cb_Connection->setCurrentText(connection);
   cb_Port->setCurrentText(port);
   cb_Baud->setCurrentText(baud);

   slotCheckValues();

   connect( cb_Connection, TQT_SIGNAL(textChanged(const TQString &)), this, TQT_SLOT(slotCheckValues(const TQString &)) );
}

void GnokiiConfig::getValues(TQString &model, TQString &connection, TQString &port, TQString &baud) const
{
   model = cb_Model->currentText();
   connection = cb_Connection->currentText();
   port = cb_Port->currentText();
   baud = cb_Baud->currentText();
}

void GnokiiConfig::slotCheckValues(const TQString &txt)
{
   bool disable_serial = (TQString("infrared irda").find(txt,0,false)>=0);
   textLabelBaudRate->setDisabled(disable_serial);
   cb_Baud->setDisabled(disable_serial);
}

void GnokiiConfig::slotCheckValues()
{
   slotCheckValues(cb_Connection->currentText());
}

#include "gnokiiconfig.moc"
