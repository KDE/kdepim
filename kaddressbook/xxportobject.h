/*
    This file is part of KAddressbook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef XXPORTOBJECT_H
#define XXPORTOBJECT_H

#include <qobject.h>

#include <kabc/addresseelist.h>
#include <kaddressbook/kabcore.h>
#include <klibloader.h>
#include <kxmlguiclient.h>

class XXPortObject : public QObject, virtual public KXMLGUIClient
{
  Q_OBJECT

  public:
    XXPortObject( KABCore *core, QObject *parent, const char *name = 0 );
    ~XXPortObject();

    /**
      Returns the unique identifier of this xxport modul, it should
      be the lowercase name of the import/export format e.g. 'vcard'
     */
    virtual QString identifier() const = 0;

    /**
      Reimplement this method if the XXPortManager shall
      pass a sorted list to @ref exportContacts().
     */
    virtual bool requiresSorting() const { return false; }

  public slots:
    /**
      Reimplement this method for exporting the contacts.
     */
    virtual bool exportContacts( const KABC::AddresseeList &list, const QString& identifier );

    /**
      Reimplement this method for importing the contacts.
     */
    virtual KABC::AddresseeList importContacts( const QString& identifier ) const;

  signals:
    /**
      Emitted whenever the export action is activated.
      The parameter contains the @ref identifier() for
      unique identification.
     */
    void exportActivated( const QString&, const QString& );

    /**
      Emitted whenever the import action is activated.
      The parameter contains the @ref identifier() for
      unique identification.
     */
    void importActivated( const QString&, const QString& );

  protected:
    /**
      Create the import action. The identifier is passed in the import slot.
     */
    void createImportAction( const QString &label, const QString &identifier = QString::null );

    /**
      Create the export action. The identifier is passed in the export slot.
     */
    void createExportAction( const QString &label, const QString &identifier = QString::null );

    /**
      Returns a pointer to the KABCore object. It's mainly used to
      register the actions.
     */
    KABCore *core() const;

  private slots:
    void slotImportActivated( const QString& );
    void slotExportActivated( const QString& );

  private:
    class XXPortObjectPrivate;
    XXPortObjectPrivate *d;
};

class XXPortFactory : public KLibFactory
{
  public:
    virtual XXPortObject *xxportObject( KABCore *core, QObject *parent,
                                        const char *name = 0 ) = 0;

  protected:
    virtual QObject* createObject( QObject*, const char*, const char*,
                                   const QStringList & )
    {
      return 0;
    }
};

#endif
