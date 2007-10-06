/*
    This file is part of KAddressbook.
    Copyright (c) 1999 Don Sanders <dsanders@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KADDRESSBOOKMAIN_H
#define KADDRESSBOOKMAIN_H


#include <kaction.h>
#include <kapplication.h>
#include <kxmlguiwindow.h>
#include <kurl.h>

class KABCore;

/**
  This class serves as the main window for KAddressBook.  It handles the
  menus, toolbars, and status bars.

  @short Main window class
  @author Don Sanders <dsanders@kde.org>
  @version 0.1
 */
class KAddressBookMain : public KXmlGuiWindow
{
  Q_OBJECT

  public:
    KAddressBookMain( const QString &file = QString() );
    virtual ~KAddressBookMain();

  public slots:
    void addEmail( QString addr );
    void importVCard( const KUrl& url );
    void importVCardFromData( const QString& vCard );
    // FIXME the below was ASYNC, but moc seems to not like that, at present
    void showContactEditor( QString uid );
    void newContact();
    QString getNameByPhone( QString phone );
    void save();
    void exit();
    bool handleCommandLine();

  protected:
    void initActions();

    /**
      This function is called when it is time for the app to save its
      properties for session management purposes.
     */
    void saveProperties( KConfigGroup& );

    /**
      This function is called when this app is restored.  The KConfig
      object points to the session management config file that was saved
      with @ref saveProperties
     */
    void readProperties( const KConfigGroup& );

    virtual bool queryClose();

  private slots:
    void configureKeyBindings();
    void configureToolbars();
    void newToolbarConfig();

  private:
    KABCore *mCore;
};

#endif
