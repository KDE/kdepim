/*
  This file is part of libkdepim.

  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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

#ifndef KCHECKCOMBOBOX_H
#define KCHECKCOMBOBOX_H

#include "kdepim_export.h"

#include <KComboBox>
#include <QModelIndex>

namespace KPIM {

/**
 * A combobox that shows its items in such a way that they can be checked in the
 * drop menu. It provides methods to set the default text when no items are selected
 * and the separator that is used to show the items that are selected in the line
 * edit.
 */
class KDEPIM_EXPORT KCheckComboBox : public KComboBox
{
    Q_OBJECT

    Q_PROPERTY( QString separator READ separator WRITE setSeparator )
    Q_PROPERTY( QString defaultText READ defaultText WRITE setDefaultText )
    Q_PROPERTY( bool squeezeText READ squeezeText WRITE setSqueezeText )
    Q_PROPERTY( QStringList checkedItems READ checkedItems WRITE setCheckedItems )

public:
    /**
     * Creates a new checkable combobox.
     *
     * @param parent The parent widget.
     */
    explicit KCheckComboBox( QWidget *parent = 0 );

    /**
     * Destroys the time zone combobox.
     */
    virtual ~KCheckComboBox();

    /**
     * Hides the popup list if it is currently shown.
     */
    virtual void hidePopup();

    /**
     * Returns the default text that is shown when no items are selected.
     */
    QString defaultText() const;

    /**
     * Sets the default text that is shown when no items are selected.
     *
     * @param text The new default text
     */
    void setDefaultText( const QString &text );

    /**
     * Returns whether the default text is always shown, even if there are
     * no checked items.
     */
    bool alwaysShowDefaultText() const;

    /**
     * Sets if the default text should always be shown even if there are
     * no checked items.
     *
     * Default is false.
     */
    void setAlwaysShowDefaultText( bool always );

    /**
     * Returns whether or not the text will be squeezed to fit in the combo's line
     * edit. This property is false by default.
     *
     * @see KSqueezedTextLabel
     */
    bool squeezeText() const;

    /**
     * Sets whether or not the text must be squeezed.
     *
     * @param squeeze The new squeeze status
     */
    void setSqueezeText( bool squeeze );

    /**
     * Return whether or not the item at @param index is enabled, i.e. if the
     * user can (un)check the item.
     */
    bool itemEnabled( int index );

    /**
     * Set the item at @param index to @param enabled, i.e. if the
     * user can (un)check the item.
     */
    void setItemEnabled( int index, bool enabled = true );

    /**
     * Returns the check state of item at given index.
     *
     * @param index The index for which to return the check state.
     */
    Qt::CheckState itemCheckState( int index ) const;

    /**
     * Changes the check state of the given index to the given state.
     *
     * @param index The index of which the state needs to be changed
     * @param state The new state
     */
    void setItemCheckState( int index, Qt::CheckState state );

    /**
     * Returns the current separator used to separate the selected items in the
     * line edit of the combo box.
     */
    QString separator() const;

    /**
     * Sets the separator used to separate items in the line edit.
     *
     * @param separator The new separator
     */
    void setSeparator( const QString &separator );

    /**
     * Returns The currently selected items.
     * @param role The role the returned values belong to.
     */
    QStringList checkedItems( int role = Qt::DisplayRole ) const;

public Q_SLOTS:
    /**
     * Sets the currently selected items. Items that are not found in the model
     * are silently ignored.
     *
     * @param items The items that will be set to checked.
     * @param role The role @p items belong to.
     */
    void setCheckedItems( const QStringList &items, int role = Qt::DisplayRole );

Q_SIGNALS:
    /**
     * Signal to notify listeners that the current selections has changed.
     *
     * @param items The new selection.
     */
    void checkedItemsChanged( const QStringList &items );

protected:
    virtual bool eventFilter( QObject *receiver, QEvent *event );
    virtual void keyPressEvent( QKeyEvent *event );
    virtual void resizeEvent( QResizeEvent * event );
#ifndef QT_NO_WHEELEVENT
    virtual void wheelEvent( QWheelEvent *event );
#endif

private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void makeInsertedItemsCheckable(const QModelIndex &, int start, int end) )
    Q_PRIVATE_SLOT( d, void updateCheckedItems( const QModelIndex &topLeft,
                                                const QModelIndex &bottomRight ) )
    Q_PRIVATE_SLOT( d, void toggleCheckState() )
    //@endcond
};

}

#endif // KCHECKCOMBOBOX_H
