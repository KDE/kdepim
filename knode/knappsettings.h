/***************************************************************************
                          knappsettings.h  -  description
                             -------------------
    
    copyright            : (C) 2000 by Christian Gebauer
    email                : gebauer@bigfoot.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef KNAPPSETTINGS_H
#define KNAPPSETTINGS_H

#include <qlistbox.h>
#include <qcolor.h>
#include "knsettingsdialog.h"

class QCheckBox;
class QListBox;


class KNAppSettings : public KNSettingsWidget  {

  Q_OBJECT  

  public:
    KNAppSettings(QWidget *p);
    ~KNAppSettings();
    
    void apply();
    
  protected:
    void init();

    //===================================================================================
    // code taken from KMail, Copyright (C) 2000 Espen Sand, espen@kde.org

    class ColorListItem : public QListBoxText {

      public:
        ColorListItem( const QString &text, const QColor &color=Qt::black );
        ~ColorListItem();
        const QColor& color()                     { return mColor; }
        void  setColor( const QColor &color )     { mColor = color; }

      protected:
        virtual void paint( QPainter * );
        virtual int height( const QListBox * ) const;
        virtual int width( const QListBox * ) const;

      private:
        QColor mColor;
    };

    //===================================================================================

    class FontListItem : public QListBoxText {

      public:
        FontListItem( const QString &name, const QFont & );
        ~FontListItem();
        const QFont& font()                     { return f_ont; }
        void setFont( const QFont &);

      protected:
        virtual void paint( QPainter * );
        virtual int width( const QListBox * ) const;

      private:
        QFont f_ont;
        QString fontInfo;
    };

    //===================================================================================
    
    QListBox *cList, *fList;
    QCheckBox *longCB, *colorCB, *fontCB;
    QButton *changeFontB, *defaultFontB, *changeColorB, *defaultColorB;

  protected slots:
    void slotColCBtoggled(bool);
    void slotColSelectionChanged();
    void slotColItemSelected(QListBoxItem *);   // show color dialog for the entry
    void slotChangeColorBtnClicked();
    void slotDefaultColorBtnClicked();

    void slotFontCBtoggled(bool);
    void slotFontSelectionChanged();
    void slotFontItemSelected(QListBoxItem *);  // show font dialog for the entry
    void slotChangeFontBtnClicked();            // show color dialog for the entry
    void slotDefaultFontBtnClicked();

};

#endif
