/*
 *   kmail: KDE mail client
 *   This file: Copyright (C) 2000 Espen Sand, espen@kde.org
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef _COLOR_LISTBOX_H_
#define _COLOR_LISTBOX_H_

#include <klistbox.h>

class ColorListBox : public KListBox
{
  Q_OBJECT

  public:
    ColorListBox( TQWidget *parent=0, const char * name=0, WFlags f=0 );
    void setColor( uint index, const TQColor &color );
    TQColor color( uint index ) const;
signals:
    void changed();

  public slots:
    virtual void setEnabled( bool state );

  protected:
    void dragEnterEvent( TQDragEnterEvent *e );
    void dragLeaveEvent( TQDragLeaveEvent *e );
    void dragMoveEvent( TQDragMoveEvent *e );
    void dropEvent( TQDropEvent *e );

  private slots:
    void newColor( int index );

  private:
    int mCurrentOnDragEnter;

};


class ColorListItem : public QListBoxItem
{
  public:
    ColorListItem( const TQString &text, const TQColor &color=Qt::black );
    const TQColor &color( void );
    void  setColor( const TQColor &color );

  protected:
    virtual void paint( TQPainter * );
    virtual int height( const TQListBox * ) const;
    virtual int width( const TQListBox * ) const;

  private:
    TQColor mColor;
    int mBoxWidth;
};

#endif

