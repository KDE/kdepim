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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KADDRESSBOOKVIEW_H
#define KADDRESSBOOKVIEW_H

#include <qstringlist.h>
#include <qwidget.h>

#include <kabc/field.h>
#include <klibloader.h>

#include "filter.h"
#include "viewconfigurewidget.h"

class KConfig;
class KXMLGUIClient;

class QDropEvent;

namespace KAB { class Core; }
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

    KAddressBookView( KAB::Core *core, QWidget *parent, const char *name );
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
     */
    // The KConfig object is unused so we do not document it
    // else doxygen will complain.
    virtual void writeConfig( KConfig * );

    /**
      Returns a QString with all the selected email addresses concatenated
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
      @return The Core object.
     */
    KAB::Core *core() const;

    /**
      @return The current sort field.
     */
    virtual KABC::Field *sortField() const = 0;

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

    /**
      Selects the first contact in the view.
     */
    virtual void setFirstSelected( bool selected = true ) = 0;

    /**
      Call this slot to popup a rmb menu.

      @param point The position where the menu shall appear.
     */
    void popup( const QPoint &point );

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

      @param uid The uid of the selected addressee

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

    /**
      This signal is emitted whenever the sort field changed.
     */
    void sortFieldChanged();

    /**
      Emitted whenever the view fields changed.
     */
    void viewFieldsChanged();

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

  private slots:
    void updateView();

  private:
    void initGUI();

    DefaultFilterType mDefaultFilterType;
    Filter mFilter;
    QString mDefaultFilterName;
    KAB::Core *mCore;
    KABC::Field::List mFieldList;

    QWidget *mViewWidget;
};

class ViewFactory : public KLibFactory
{
  public:
    virtual KAddressBookView *view( KAB::Core *core, QWidget *parent,
                                    const char *name = 0 ) = 0;
    /**
      @return The type of the view. This is normally a small one word
      string (ie: Table, Icon, Tree, etc).
     */
    virtual QString type() const = 0;

    /**
      @return The description of the view. This should be a 3 to
      4 line string (don't actually use return characters in the string)
      describing the features offered by the view.
     */
    virtual QString description() const = 0;

    /**
      Creates a config dialog for the view type. The default
      implementation will return a ViewConfigDialog. This default
      dialog will allow the user to set the visible fields only. If
      you need more config options (as most views will), this method
      can be overloaded to return your sublcass of ViewConfigDialog.
      If this method is over loaded the base classes method should
      <B>not</B> be called.
     */
    virtual ViewConfigureWidget *configureWidget( KABC::AddressBook *ab,
                                                  QWidget *parent,
                                                  const char *name = 0 );

  protected:
    virtual QObject* createObject( QObject*, const char*, const char*,
                                   const QStringList & )
    {
      return 0;
    }
};

#endif
