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

#ifndef MODELCOLUMNMANAGER_H
#define MODELCOLUMNMANAGER_H

#include "contactstreemodel.h"

#include <QtCore/QObject>

class QWidget;

/**
 * @short A manager for the contacts model columns.
 *
 * This class manages which columns shall be provided by the
 * contacts model. It keeps track of the configuration file
 * and the user configuration.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class ModelColumnManager : public QObject
{
  Q_OBJECT
  public:
    /**
     * Creates a new model column manager.
     *
     * @param model The model that shall be managed.
     * @param parent The parent object.
     */
    explicit ModelColumnManager( Akonadi::ContactsTreeModel *model, QObject *parent = 0 );

    /**
     * Loads the user configuration and applies it to the model.
     */
    void load();

    /**
     * Stores the user configuration.
     */
    void store();

    /**
     * Sets the widget that shall provide a RMB menu to
     * configure the columns to be shown.
     */
    void setWidget( QWidget *view );

  protected:
    virtual bool eventFilter( QObject *watched, QEvent* event );

  private Q_SLOTS:
    void adaptHeaderView();

  private:
    Akonadi::ContactsTreeModel *mModel;
    Akonadi::ContactsTreeModel::Columns mColumns;
    QWidget *mWidget;
};

#endif
