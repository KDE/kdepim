/*
  This file is part of KAddressBook.
  Copyright (c) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>
                     Tobias Koenig <tokoe@kde.org>

  Copyright (c) 2009 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef STYLEPAGE_H
#define STYLEPAGE_H

#include <QWidget>
#include "contactfields.h"

class QLabel;
class QPixmap;
class KComboBox;

class StylePage : public QWidget
{
  Q_OBJECT

  public:
    explicit StylePage( QWidget *parent = 0, const char *name = 0 );
    ~StylePage();

    /**
     * Set a preview image. If @ref pixmap is 'null' a text will
     * be displayed instead.
     */
    void setPreview( const QPixmap &pixmap );

    /**
     * Add a style name.
     */
    void addStyleName( const QString &name );

    /**
     * Clear the style name list.
     */
    void clearStyleNames();

    /**
     * Set the sort criterion field.
     */
    void setSortField( ContactFields::Field field );

    /**
     * Returns the sort criterion field.
     */
    ContactFields::Field sortField() const;

    /**
     * Sets the sort order.
     */
    void setSortOrder( Qt::SortOrder sortOrder );

    /**
     * Returns the sort order.
     */
    Qt::SortOrder sortOrder() const;

    /**
     * Returns the sort order.
     */
    int printingStyle() const;

    /**
     * Returns the sort order.
     */
    void setPrintingStyle( int index );

  Q_SIGNALS:
    /**
     * This signal is emmited when the user selects a new style in the
     * style combo box.
     */
    void styleChanged( int index );

  private:
    void initGUI();
    void initFieldCombo();

    KComboBox *mFieldCombo;
    KComboBox *mSortTypeCombo;
    KComboBox *mStyleCombo;
    QLabel *mPreview;

    ContactFields::Fields mFields;
};

#endif // STYLEPAGE_H
