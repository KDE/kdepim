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

#ifndef KNODE_RANGEFILTER_H
#define KNODE_RANGEFILTER_H

#include <QGroupBox>

class QLabel;
class KIntSpinBox;
class QComboBox;
class QCheckBox;

class KSimpleConfig;

namespace KNode {

/** Filter for interger ranges. */
class RangeFilter
{
  friend class RangeFilterWidget;

  public:
    RangeFilter()   { op1=eq; op2=dis; val1=0; val2=0; enabled=false; }
    ~RangeFilter()  {}

    RangeFilter& operator=( const RangeFilter &nr )
      { val1=nr.val1; val2=nr.val2;
        op1=nr.op1; op2=nr.op2;
        enabled=nr.enabled;
        return (*this); }

    void load(KSimpleConfig *conf);
    void save(KSimpleConfig *conf);

    bool doFilter(int a);

  protected:
    enum Op { gt=0, gtoeq=1, eq=2, ltoeq=3, lt=4, dis=5 };
    bool matchesOp(int v1, Op o, int v2);

    int val1, val2;
    Op op1, op2;
    bool enabled;

};


//==================================================================================


/** Configuration widget for KNode::RangeFilter. */
class RangeFilterWidget : public QGroupBox
{
  Q_OBJECT

  public:
    /** Creates a new RangeFilter configuration widet.
     * @param value The name of the value that is filtered.
     * @param min The minimum filter range.
     * @param max The maximum filter range.
     * @param parent The parent widget.
     * @param unit Unit of the filtered value.
     */
    RangeFilterWidget( const QString& value, int min, int max, QWidget* parent,
                       const QString &unit = QString::null );
    ~RangeFilterWidget();

    RangeFilter filter();
    void setFilter( RangeFilter &f );
    void clear();

  protected:
    QCheckBox *enabled;
    QLabel *des;
    KIntSpinBox *val1, *val2;
    QComboBox *op1, *op2;

  protected slots:
    void slotEnabled(bool e);
    void slotOp1Changed(int id);
    void slotOp2Changed(int id);

};

}

#endif
