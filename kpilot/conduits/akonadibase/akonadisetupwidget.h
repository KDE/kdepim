#ifndef AKONADISETUPWIDGET_H
#define AKONADISETUPWIDGET_H
/* akonadisetupwidget.h                       KPilot
**
** Copyright (C) 2008 Bertjan Broeksema
**
** This file defines the widget and behavior for the config dialog
** of the Contacts conduit.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "ui_akonadi-setup-widget.h"

namespace Akonadi
{
	class Collection;
};

class AkonadiSetupWidgetPrivate;

class AkonadiSetupWidget : public QWidget
{
Q_OBJECT

public:
	AkonadiSetupWidget( const QStringList& mimeTypes, qint64 initialId );
	
	~AkonadiSetupWidget();

	qint64 currentCollectionId() const;

	/**
	 * Returns true when the collection is changed by selecting another collection
	 * from the combobox.
	 */
	bool collectionChanged() const;

private slots:
	void changeCollection( const Akonadi::Collection& collection );

private:
	QSharedDataPointer<AkonadiSetupWidgetPrivate> d;
};

#endif

