/*                                                                      
    This file is part of KAddressBook.
    Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>
                                                                        
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

#ifndef LOOK_KABBASIC_H
#define LOOK_KABBASIC_H

#include <kabc/addressbook.h>
#include <qwidget.h>

class KConfig;

/**
  This is a pure virtual base class that defines the
  interface for how to display and change entries of
  the KDE addressbook.

  This basic widget does not show anything in its client space.
  Derive it and implement its look and how the user may edit the
  entry.

  The paintEvent() has to paint the whole widget, since repaint()
  calls will not delete the widgets background.
 */
class KABBasicLook : public QWidget
{
  Q_OBJECT

  public:
    /**
      The constructor.
     */
    KABBasicLook( QWidget *parent = 0, const char *name = 0 );

    /**
      Set the entry. It will be displayed automatically.
     */
    virtual void setAddressee( const KABC::Addressee& addressee );

    /**
      Get the current entry.
     */
    virtual KABC::Addressee addressee();

    /**
      Configure the view from the configuration file.
     */
    virtual void restoreSettings( KConfig* );

    /**
      Save the view settings to the configuration file.
     */
    virtual void saveSettings( KConfig* );

    /**
      Retrieve read-write state.
     */
    bool isReadOnly() const;

  signals:
    /**
      This signal is emitted when the user changed the entry.
     */
    void entryChanged();

    /**
      This signal indicates that the entry needs to be changed
      immidiately in the database. This might be due to changes in
      values that are available in menus.
     */
    void saveMe();

    /**
      The user acticated the email address displayed. This may happen
      by, for example, clicking on the displayed mailto-URL.
     */
    void sendEmail( const QString &email );

    /**
      The user activated one of the displayed HTTP URLs. For example
      by clicking on the displayed homepage address.
     */
    void browse( const QString &url );

  public slots:
    /**
      Set read-write state.
     */
    virtual void setReadOnly( bool state );

  private:
    KABC::Addressee mAddressee;
    bool mReadOnly;
};

class KABLookFactory
{
  public:
    KABLookFactory( QWidget *parent = 0, const char *name = 0 );
    virtual ~KABLookFactory();

    virtual KABBasicLook *create() = 0;

    /**
      Overload this method to provide a one-liner description
      for your look.
     */
    virtual QString description() = 0;

  protected:
    QWidget *mParent;
    const char* mName;
};

#endif
