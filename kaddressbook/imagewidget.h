/*                                                                      
    This file is part of KAddressBook.                                  
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

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <qwidget.h>

#include <kabc/picture.h>
#include <kdialogbase.h>

class KURLRequester;

class QCheckBox;
class QLabel;

class ImageWidget : public QWidget
{
  Q_OBJECT

  public:
    ImageWidget( bool readOnly, QWidget *parent, const char *name = 0 );
    ~ImageWidget();

    /**
      Sets the photo object.
     */
    void setPhoto( const KABC::Picture &photo );

    /**
      Returns a photo object.
     */
    KABC::Picture photo() const;

    /**
      Sets the logo object.
     */
    void setLogo( const KABC::Picture &photo );

    /**
      Returns a logo object.
     */
    KABC::Picture logo() const;

  signals:
    void changed();

  private slots:
    void loadPhoto();
    void loadLogo();
    void updateGUI();

  private:
    QPixmap loadPixmap( const KURL &url );

    KURLRequester *mPhotoUrl;
    KURLRequester *mLogoUrl;

    QCheckBox *mUsePhotoUrl;
    QCheckBox *mUseLogoUrl;
    QLabel *mPhotoLabel;
    QLabel *mLogoLabel;

    bool mReadOnly;
};

#endif
