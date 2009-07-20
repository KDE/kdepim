#ifndef AKONADISETUPWIDGET_H
#define AKONADISETUPWIDGET_H
/* akonadisetupwidget.h                       KPilot
**
** Copyright (C) 2008 Bertjan Broeksema <b.broeksema@kdemail.net>
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

#include <akonadi/collection.h>
#include <akonadi/item.h>

#include "options.h"
#include "ui_akonadi-setup-widget.h"

class KPILOT_EXPORT AkonadiSetupWidget : public QWidget
{
Q_OBJECT

public:
	AkonadiSetupWidget( QWidget* parent = 0 );

	~AkonadiSetupWidget();

	/**
	 * Returns the current selected collection.
	 */
	Akonadi::Item::Id collection() const;

	/**
	 * Returns true when the user selected a collection using the combobox.
	 */
	bool modified() const;
	
	/**
	 * Sets the current selected collection, but doesn't change the status.
	 */
	void setCollection( Akonadi::Item::Id id );

	/**
	 * Sets the label text for the collection combobox.
	 */
	void setCollectionLabel( const QString& label );

	/**
	 * Sets the mimetypes for the collections combobox. Only collections which
	 * have one of the mimetypes set, will be shown.
	 */
	void setMimeTypes( const QStringList& mimeTypes );

signals:
	void collectionChanged();

private slots:
	void changeCollection( const Akonadi::Collection& collection );

private:
	class Private;
	Private* d;
	Akonadi::Item::Id selectedId;
};

#endif

