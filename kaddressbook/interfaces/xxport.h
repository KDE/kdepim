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

#ifndef KAB_XXPORT_H
#define KAB_XXPORT_H

#include <qobject.h>

#include <kabc/addressbook.h>
#include <kabc/addresseelist.h>
#include <klibloader.h>
#include <kxmlguiclient.h>
#include <kdepimmacros.h>

#define KAB_XXPORT_PLUGIN_VERSION 1

class KApplication;

/**
  K_EXPORT_KADDRESSBOOK_XXFILTER_CATALOG() creates the stub for a KAddressbook import/export filter.
  @libname	filename of the shared library, e.g. libkaddrbk_bookmark_xxport
  @XXPortClass	the import/export class - derived from the XXPort class
  @catalog	catalog file to search for translation lookup (NULL if no catalog needed)
  @see: K_EXPORT_COMPONENT_FACTORY()
 */
#define K_EXPORT_KADDRESSBOOK_XXFILTER_CATALOG( libname, XXPortClass, catalog ) \
 class KDE_NO_EXPORT localXXPortFactory : public KAB::XXPortFactory { \
	KAB::XXPort *xxportObject( KABC::AddressBook *ab, QWidget *parent, const char *name ) \
	 { const char *cat = catalog; \
	   if (cat) KGlobal::locale()->insertCatalogue(cat); \
	   return new XXPortClass( ab, parent, name ); \
         } \
 }; \
 K_EXPORT_COMPONENT_FACTORY( libname, localXXPortFactory )

/**
  K_EXPORT_KADDRESSBOOK_XXFILTER() creates the stub for a KAddressbook import/export filter.
  @libname	filename of the shared library, e.g. libkaddrbk_bookmark_xxport
  @XXPortClass	the import/export class - derived from the XXPort class
  @see: K_EXPORT_COMPONENT_FACTORY()
 */
#define K_EXPORT_KADDRESSBOOK_XXFILTER( libname, XXPortClass ) \
	K_EXPORT_KADDRESSBOOK_XXFILTER_CATALOG( libname, XXPortClass, NULL )
	

namespace KAB {

class XXPort : public QObject, virtual public KXMLGUIClient
{
  Q_OBJECT

  public:
    XXPort( KABC::AddressBook *ab, QWidget *parent, const char *name = 0 );
    ~XXPort();

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

    /**
      set the KApplication pointer.
      @see: processEvents()
     */
    void setKApplication( KApplication *app );

    /**
      Processes outstanding KApplication events. It should be called 
      occasionally when the import/export filter is busy performing
      a long operation (e.g. reading from slow external devices).
      @see: QApplication::processEvents()
     */
    void processEvents() const;

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
      Returns a pointer to the address book object.
     */
    KABC::AddressBook *addressBook() const;

    /**
      Returns a pointer to the parent widget. It can be used as parent for
      message boxes.
     */
    QWidget *parentWidget() const;

  private slots:
    void slotImportActivated( const QString& );
    void slotExportActivated( const QString& );

  private:
    KABC::AddressBook *mAddressBook;
    QWidget *mParentWidget;

    class XXPortPrivate;
    XXPortPrivate *d;
};

class XXPortFactory : public KLibFactory
{
  public:
    virtual XXPort *xxportObject( KABC::AddressBook *ab, QWidget *parent,
                                  const char *name = 0 ) = 0;

  protected:
    virtual QObject* createObject( QObject*, const char*, const char*,
                                   const QStringList & )
    {
      return 0;
    }
};


} /* namespace KAB */

#endif
