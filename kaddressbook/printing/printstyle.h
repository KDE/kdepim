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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#ifndef PRINTSTYLE_H
#define PRINTSTYLE_H

#include <qwidget.h>
#include <qstringlist.h>
#include <qpixmap.h>

#include <kabc/field.h>

namespace KABPrinting {

class PrintingWizard;
class PrintProgress;

/**
 The class PrintStyle implements the abstract interface to the
 PrintingWizards style objects.
 To implement a print style, derive from this class and read
 the information in printingwizard.h to see how this two pieces
 work together. Basically, the print style gets the contacts it
 is supposed to print from the PrintingWizard is will not
 change this set - neither its content nor its order.
 To register your new style in the printing wizard, you need to
 define a PrintStyleFactory that handles how your objects are
 created and deleted. See the existing print styles for
 examples.
 A print style should have a preview image that gives the user
 a basic impression on how it will look. Add this image to the
 printing folder (right here :-), and edit Makefile.am to have
 it installed along with kaddressbook. Load it using
 setPreview(QString).
 Your print style is supposed to add its options as pages to
 the printing wizard. The method wizard() gives you a pointer
 to the wizard object.
 */

class PrintStyle : public QObject
{
  Q_OBJECT

  public:
    PrintStyle( PrintingWizard* parent, const char* name = 0 );
    virtual ~PrintStyle();

    /**
     Reimplement this method to actually print.
     */
    virtual void print( const KABC::Addressee::List &contacts, PrintProgress* ) = 0;

    /**
     Reimplement this method to provide a preview of what will
     be printed. It returns an invalid QPixmap by default,
     resulting in a message that no preview is available.
     */
    const QPixmap& preview();

    /**
     Hide all style specific pages in the wizard.
     */
    void hidePages();

    /**
     Show all style specific pages in the wizard.
     */
    void showPages();

    /**
      Returns the preferred sort criterion field.
     */
    KABC::Field* preferredSortField();

    /**
      Returns the preferred sort type.
      
      true = ascending
      false = descending
     */
    bool preferredSortType();

  protected:
    /**
     Load the preview image from the kaddressbook data
     directory. The image should be located in the subdirectory
     "printing". Give only the file name without any prefix as
     the parameter.
     */
    bool setPreview( const QString& fileName );

    /**
     Set the preview image.
     */
    void setPreview( const QPixmap& image );

    /**
      Set preferred sort options for this printing style.
     */
    void setPreferredSortOptions( KABC::Field *field, bool ascending = true );

    /**
     Return the wizard object.
     */
    PrintingWizard *wizard();

    /**
     Add additional page to the wizard e.g. a configuration page for
     the style.
     */
    void addPage( QWidget *page, const QString &title );

  private:
    PrintingWizard *mWizard;
    QPixmap mPreview;
    QPtrList<QWidget> mPageList;
    QStringList mPageTitles;

    KABC::Field *mSortField;
    bool mSortType;
};


/**
  The factories are used to have all object of the respective
  print style created in one place.
  This will maybe be changed to a template because of its simple
  nature :-)
*/
class PrintStyleFactory
{
  public:
    PrintStyleFactory( PrintingWizard* parent, const char* name = 0 );
    virtual ~PrintStyleFactory();

    virtual PrintStyle *create() const = 0;

    /**
      Overload this method to provide a one-liner description
      for your print style.
     */
    virtual QString description() const = 0;

  protected:
    PrintingWizard* mParent;
    const char* mName;
};

}

#endif
