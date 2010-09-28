/*
    This file is part of KAddressBook.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef XXPORTMANAGER_H
#define XXPORTMANAGER_H

#include "xxport/xxportfactory.h"

#include <akonadi/collection.h>

#include <QtCore/QObject>

class QAbstractItemModel;
class QAction;
class QItemSelectionModel;
class QSignalMapper;

class KJob;
class KProgressDialog;

/**
 * @short The class that manages import and export of contacts.
 */
class XXPortManager : public QObject
{
  Q_OBJECT

  public:
    /**
     * Creates a new xxport manager.
     *
     * @param parent The widget that is used as parent for dialogs.
     */
    XXPortManager( QWidget *parent = 0 );

    /**
     * Destroys the xxport manager.
     */
    ~XXPortManager();

    /**
     * Adds a new action to import contacts.
     *
     * @param action The action object.
     * @param identifier The identifier that will be passed to the xxport module.
     */
    void addImportAction( QAction *action, const QString &identifier );

    /**
     * Adds a new action to export contacts.
     *
     * @param action The action object.
     * @param identifier The identifier that will be passed to the xxport module.
     */
    void addExportAction( QAction *action, const QString &identifier );

    /**
     * Sets the @p model that contains the current selection.
     *
     * @note This model is used by the ContactSelectionDialog.
     */
    void setSelectionModel( QItemSelectionModel *model );

  public Q_SLOTS:
    /**
     * Sets the @p addressBook that shall be preselected in the
     * ContactSelectionDialog.
     */
    void setDefaultAddressBook( const Akonadi::Collection &addressBook );

  private Q_SLOTS:
    void slotImport( const QString& );
    void slotExport( const QString& );

    void slotImportJobDone( KJob* );

  private:
    QItemSelectionModel *mSelectionModel;
    QWidget *mParentWidget;
    XXPortFactory mFactory;
    QSignalMapper *mImportMapper;
    QSignalMapper *mExportMapper;
    Akonadi::Collection mDefaultAddressBook;
    KProgressDialog *mImportProgressDialog;
};

#endif
