/*    
	imaddresswidget.h
	
	IM address editor widget for KAddressbook
	
	Copyright (c) 2004 Will Stephenson   <lists@stevello.free-online.co.uk>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef IMADDRESSWIDGET_H
#define IMADDRESSWIDGET_H

#include "imaddressbase.h"

#include "imeditorwidget.h"

class IMAddressWidget : public IMAddressBase
{
public:
	// VCard support disabled pending standard vcard IM address storage
	IMAddressWidget( QWidget *parent );
	IMAddressWidget( QWidget *parent, const IMProtocol& protocol, const QString& address, const IMContext& context/*, bool inVCard*/ );
	IMProtocol protocol();
	IMContext context();
	QString address();
	//bool inVCard();
protected:
	/**
	 * Populate combobox with protocols
	 */
	void populateProtocols();
};

#endif
