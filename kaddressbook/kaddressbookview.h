/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KADDRESSBOOKVIEW_H
#define KADDRESSBOOKVIEW_H

#include <qstringlist.h>
#include <qwidget.h>

#include <kabc/field.h>

#include "filter.h"
#include "viewmanager.h"

class QDropEvent;
class KConfig;

namespace KABC { class AddressBook; }

/**
  Base class for all views in kaddressbook. This class implements
  all the common methods needed to provide a view to the user.
 
  To implement a specific view (table, card, etc), just inherit from
  this class and implement all the pure virtuals.
 
  @author Mike Pilone <mpilone@slac.com>
 */
class KAddressBookView : public QWidget
{
  Q_OBJECT

  public:
    enum DefaultFilterType { None = 0, Active = 1, Specific = 2 };
    
    KAddressBookView( KABC::AddressBook *ab, QWidget *parent, const char *name );
    virtual ~KAddressBookView();

    /**
      Must be overloaded in subclasses. Should return a list of
      all the uids of selected contacts.
     */
    virtual QStringList selectedUids() = 0;

    /**
      Called whenever this view should read the config. This can be used
      as a sign that the config has changed, therefore the view should
      assume the worst and rebuild itself if necessary. For example,
      in a table view this method may be called when the user adds or
      removes columns from the view.

      If overloaded in the subclass, do not forget to call super class's
      method.

      @param config The KConfig object to read from. The group will already
      be set, so do not change the group.
     */
    virtual void readConfig( KConfig *config );

    /**
      Called whenever this view should write the config. The view should not
      write out information handled by the application, such as which fields
      are visible. The view should only write out information specific
      to itself (i.e.: All information in the ViewConfigWidget)

      If overloaded in the subclass, do not forget to call the super class's
      method.

      @param config The KConfig object to read from. The group will already
      be set, so do not change the group.
     */
    virtual void writeConfig( KConfig *config );

    /**
      Returns a QString with all the selected email addresses contactenated
      together with a ',' seperator.
     */
    virtual QString selectedEmails();

    /**
      Return the type of the view: Icon, Table, etc. Please make sure that
      this is the same value that ViewWrapper::type() will return for your
      view.
     */
    virtual QString type() const = 0;

    /**
      This method can be overloaded in the subclass to implement incremental
      searching. Incremental searching is where the user types in characters
      and after each character the selection is updated to select the
      item (addressee) that matches the search.

      For example, if this method is called with <i>value</i> being 'p', the
      selection should be moved to the first item with the <i>field</i>
      matching 'p'. If the user then typed 'r', making the value 'pr', the
      selection should be updated to the first item with the <i>field</i>
      matching 'pr'. If there is no item matching 'pr', the selection should
      remain on the last valid match. If the <i>value</i> is empty,
      select the first item.

      @param value The string value to match.
      @param field The KABC::Field that should be searched
     */
    virtual void incrementalSearch( const QString &value, KABC::Field *field );

    /**
      Returns a list of the fields that should be displayed. The list
      is composed of the fields proper names (ie: Home Address), so
      the view may need to translate them in order to get the
      value from the addressee.
    
      This list is generated from the config file, so it is advisable to call
      this method whenever a readConfig() is called in order to get the newest
      list of fields.
     */
    KABC::Field::List fields() const;
    
    /**
      Sets the active filter. This filter will be used for filtering
      the list of addressees to display. The view will <b>not</b>
      automatically refresh itself, so in most cases you will want to call
      KAddressBookView::refresh() after this method.
     */
    void setFilter( const Filter& );
    
    /**
      @return The default filter type selection. If the selection
      is SpecificFilter, the name of the filter can be retrieved with
      defaultFilterName()
     */
    DefaultFilterType defaultFilterType() const;
    
    /**
      @return The name of the default filter. This string is
      only valid if defaultFilterType() is returning SpecificFilter.
     */
    const QString &defaultFilterName() const;
    
    /**
      @return The address book.
     */
    KABC::AddressBook *addressBook() const;

  public slots:
    /**
      Must be overloaded in subclasses to refresh the view.
      Refreshing includes updating the view to ensure that only items
      in the document are visible. If <i>uid</i> is valid, only the
      addressee with uid needs to be refreshed. This is an optimization
      only.
     */
    virtual void refresh( QString uid = QString::null ) = 0;

    /**
      This method must be overloaded in subclasses. Select (highlight)
      the addressee matching <i>uid</i>. If uid
      is equal to QString::null, then all addressees should be selected.
     */
    virtual void setSelected( QString uid = QString::null, bool selected = true ) = 0;

  signals:
    /**
      This signal should be emitted by a subclass whenever an addressee
      is modified.
     */
    void modified();

    /**
      This signal should be emitted by a subclass whenever an addressee
      is selected. Selected means that the addressee was given the focus.
      Some widgets may call this 'highlighted'. The view is responsible for
      emitting this signal multiple times if multiple items are selected,
      with the last item selected being the last emit.
    
      @param uid The uid of the selected addressee.
    
      @see KListView
     */
    void selected( const QString &uid );

    /**
      This signal should be emitted by a subclass whenever an addressee
      is executed. This is defined by the KDE system wide config, but it
      either means single or doubleclicked.
    
      @param ui The uid of the selected addressee
    
      @see KListView
     */
    void executed( const QString &uid );
    
    /**
      This signal is emitted whenever a user attempts to start a drag
      in the view. The slot connected to this signal would usually want
      to create a QDragObject.
     */
    void startDrag();
    
    /**
      This signal is emitted whenever the user drops something on the
      view. The individual view should handle checking if the item is
      droppable (ie: if it is a vcard).
     */
    void dropped( QDropEvent* );

  protected:
    /**
      Returns a list of the addressees that should be displayed. This method
      should always be used by the subclass to get a list of addressees. This
      method internally takes many factors into account, including the current
      filter.
     */
    KABC::Addressee::List addressees();

    /**
      This method returns the widget that should be used as the parent for
      all view components. By using this widget as the parent and not
      'this', the view subclass has the option of placing other widgets
      around the view (ie: search fields, etc). Do not delete this widget!
     */
    QWidget *viewWidget();

  private:
    void initGUI();

    DefaultFilterType mDefaultFilterType;
    Filter mFilter;
    QString mDefaultFilterName;
    KABC::AddressBook *mAddressBook;
    KABC::Field::List mFieldList;
    
    QWidget *mViewWidget;
};

#endif
