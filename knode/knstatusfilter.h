/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNODE_STATUSFILTER_H
#define KNODE_STATUSFILTER_H

#include <qcombobox.h>
#include <qbitarray.h>

class QCheckBox;
class KSimpleConfig;
class KNRemoteArticle;

namespace KNode {

/** Filter for article status flags. */
class StatusFilter
{
  friend class StatusFilterWidget;

  public:
    StatusFilter();
    ~StatusFilter();

    StatusFilter& operator=( const StatusFilter &sf )
      { for(int i=0; i<8; i++) data.setBit(i, sf.data.at(i)); return (*this); }

    void load(KSimpleConfig *conf);
    void save(KSimpleConfig *conf);

    bool doFilter(KNRemoteArticle *a);

  protected:
    QBitArray data;

};


//=================================================================================


/** Configuration widget for KNode::StatusFilter. */
class StatusFilterWidget : public QWidget
{
  Q_OBJECT

  public:
    StatusFilterWidget( QWidget *parent );
    ~StatusFilterWidget();

    StatusFilter filter();
    void setFilter( StatusFilter &f );
    void clear();


  protected:

    /** Combobox to select a boolean value (true/false). */
    class TFCombo : public QComboBox {

      public:
        TFCombo(QWidget *parent);
        ~TFCombo();
        void setValue(bool b) { if(b) setCurrentItem(0); else setCurrentItem(1); }
        bool value() const         { return (currentItem()==0); }
    };


    QCheckBox *enR, *enN, *enUS, *enNS;
    TFCombo *rCom, *nCom, *usCom, *nsCom;

  protected slots:
    void slotEnabled();

};

}

#endif
