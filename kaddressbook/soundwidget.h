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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#ifndef SOUNDWIDGET_H
#define SOUNDWIDGET_H

#include <qwidget.h>

#include <kabc/sound.h>

class KURLRequester;

class QCheckBox;
class QPushButton;

class SoundWidget : public QWidget
{
  Q_OBJECT

  public:
    SoundWidget( QWidget *parent, const char *name = 0 );
    ~SoundWidget();

    /**
      Sets the sound object.
     */
    void setSound( const KABC::Sound &sound );

    /**
      Returns the sound object.
     */
    KABC::Sound sound() const;

  signals:
    void changed();

  private slots:
    void playSound();
    void loadSound();
    void updateGUI();

  private:
    KURLRequester *mSoundUrl;

    QCheckBox *mUseSoundUrl;
    QPushButton *mPlayButton;

    KABC::Sound mSound;
};

#endif
