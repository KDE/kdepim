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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include <qstring.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qcombobox.h>

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


GnokiiConfig::GnokiiConfig( QWidget* parent, const char* name, bool modal, WFlags fl )
	: GnokiiConfigUI(parent, name, modal, fl)
{
   QStringList list = QStringList::split(" ", MODELS);
   cb_Model->insertStringList(list);

   list = QStringList::split(" ", CONNECTIONS);
   cb_Connection->insertStringList(list);

   list = QStringList::split(" ", AVAILABLE_PORTS);
   cb_Port->insertStringList(list);

   list = QStringList::split(" ", BAUDRATES);
   cb_Baud->insertStringList(list);
}

GnokiiConfig::~GnokiiConfig()
{
}

void GnokiiConfig::setValues(const QString &model, const QString &connection, const QString &port, const QString &baud)
{
   cb_Model->setCurrentText(model);
   cb_Connection->setCurrentText(connection);
   cb_Port->setCurrentText(port);
   cb_Baud->setCurrentText(baud);

   slotCheckValues();

   connect( cb_Connection, SIGNAL(textChanged(const QString &)), this, SLOT(slotCheckValues(const QString &)) );
}

void GnokiiConfig::getValues(QString &model, QString &connection, QString &port, QString &baud) const
{
   model = cb_Model->currentText();
   connection = cb_Connection->currentText();
   port = cb_Port->currentText();
   baud = cb_Baud->currentText();
}

void GnokiiConfig::slotCheckValues(const QString &txt)
{
   bool disable_serial = (QString("infrared irda").find(txt,0,false)>=0);
   textLabelBaudRate->setDisabled(disable_serial);
   cb_Baud->setDisabled(disable_serial);
}

void GnokiiConfig::slotCheckValues()
{
   slotCheckValues(cb_Connection->currentText());
}

#include "gnokiiconfig.moc"
