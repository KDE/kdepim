/*
    This file is part of KAddressbook.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#ifndef KAB_CONTACTEDITORWIDGET_H
#define KAB_CONTACTEDITORWIDGET_H

#include <qwidget.h>

#include <kabc/addressbook.h>
#include <klibloader.h>
#include <kdepimmacros.h>

#define KAB_CEW_PLUGIN_VERSION 1

namespace KAB {

class KDE_EXPORT ContactEditorWidget : public QWidget
{
  Q_OBJECT

  public:
    typedef QValueList<ContactEditorWidget*> List;

    ContactEditorWidget( KABC::AddressBook *ab, QWidget *parent, const char *name = 0 );
    ~ContactEditorWidget();

    /**
      Return logical height of the widget. This is used to calculate how
      much vertical space relative to other widgets this widget will use
      in the summary view.
    */
    virtual int logicalHeight() const { return 1; }

    /**
      Return logical wide of the widget. This is used to calculate how
      much horizontal space relative to other widgets this widget will use
      in the summary view.
    */
    virtual int logicalWidth() const { return 1; }

    /**
      Load the contacts data into the GUI.
     */
    virtual void loadContact( KABC::Addressee *addr ) = 0;

    /**
      Save the data from the GUI into the passed contact
      object.
     */
    virtual void storeContact( KABC::Addressee *addr ) = 0;

    /**
      Sets whether the contact should be presented as
      read-only. You should update your GUI in the reimplemented
      method.
     */
    virtual void setReadOnly( bool readOnly ) = 0;

    /**
      Returns whether this widget was modified.
     */
    bool modified() const;

  signals:
    /**
      Emitted whenever the page has changed, do not emit it directly,
      use setModified() instead.
     */
    void changed();

  public slots:
    /**
      Call this slot whenever the data were changed by the user. It
      will emit the changed() signal and set the modified property.

      @param modified Set whether the widget was modified.
     */
    void setModified( bool modified );

    void setModified();

  protected:
    /**
      Returns a pointer to the address book object.
     */
    KABC::AddressBook *addressBook() const;

  private:
    KABC::AddressBook *mAddressBook;
    bool mModified;

    class ContactEditorWidgetPrivate;
    ContactEditorWidgetPrivate *d;
};

class ContactEditorWidgetFactory : public KLibFactory
{
  public:
    virtual ContactEditorWidget *createWidget( KABC::AddressBook *ab, QWidget *parent,
                                               const char *name = 0 ) = 0;

    /**
      Returns the i18ned title of this tab page.
     */
    virtual QString pageTitle() const { return ""; }

    /**
      Returns the identifier of the tab page where the widget
      shall belong to.
     */
    virtual QString pageIdentifier() const = 0;

  protected:
    virtual QObject* createObject( QObject*, const char*, const char*,
                                   const QStringList & )
    {
      return 0;
    }
};

}

#endif
