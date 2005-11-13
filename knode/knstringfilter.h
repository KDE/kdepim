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

#ifndef KNODE_STRINGFILTER_H
#define KNODE_STRINGFILTER_H

#include <QGroupBox>

class QCheckBox;
class QComboBox;

class KLineEdit;
class KSimpleConfig;

class KNGroup;

namespace KNode {

/** Filter for string values. */
class StringFilter
{
  friend class StringFilterWidget;

  public:
    StringFilter()  { con=true; regExp=false;}
    ~StringFilter() {}

    StringFilter& operator=( const StringFilter &sf );
    /** replace placeholders */
    void expand(KNGroup *g);

    void load(KSimpleConfig *conf);
    void save(KSimpleConfig *conf);

    bool doFilter(const QString &s);

  protected:
    QString data, expanded;
    bool con, regExp;

};


//===============================================================================


/** Configuration widget for KNode::StringFilter. */
class StringFilterWidget : public QGroupBox
{
  Q_OBJECT

  public:
    /** Create a new configuration widget for StringFilter.
     * @param title Name of the value that is filtered.
     * @param parent The parent widget.
     */
    StringFilterWidget( const QString& title, QWidget *parent );
    ~StringFilterWidget();

    StringFilter filter();
    void setFilter( StringFilter &f );
    void clear();

    /** usablity hack for the search dialog */
    void setStartFocus();

  protected:
    QCheckBox *regExp;
    QComboBox *fType;
    KLineEdit *fString;

};

}

#endif

