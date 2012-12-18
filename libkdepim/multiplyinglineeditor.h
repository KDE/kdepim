/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
    Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    Refactored from earlier code by:
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef MULTIPLYINGLINEEDITOR_H
#define MULTIPLYINGLINEEDITOR_H

#include "kdepim_export.h"

#include "multiplyingline.h"

#include <KGlobalSettings>
#include <QWidget>
#include <QObject>

namespace KPIM {

class MultiplyingLineView;

/**
  @short An Abstract Base Class used to create MultiplyingLines
  Subclass this class and MultiplyingLine, then implement newLine() such that it allocates
  and returns an instance of your MultiplyingLine.
 */
class KDEPIM_EXPORT MultiplyingLineFactory : public QObject
{
  Q_OBJECT
  public:
    explicit MultiplyingLineFactory( QObject* parent ) : QObject( parent ) {}
    virtual ~MultiplyingLineFactory() {}
    virtual MultiplyingLine* newLine(  QWidget *parent ) = 0;
};

/**
  @short An editor that adds rows (lines) of widgets and deletes them as the user edits

  Line widgets in the MultiplyingLineEditor are usually composed of multiple
  basic widgets. An example is below:

  -------------------------------------------------
  | ComboBox|   Line edit             |  Checkbox |  <-- 1 line
  -------------------------------------------------
  | ComboBox|   Line edit             |  Checkbox | <-- another line

  Default behavior is one line with default settings, and when
  the user edits it, another line is automatically added.
  Lines are added and deleted on demand.

  Implement this class and MultiplyingLineData. Then implement
  MultiplyingLineFactory to return instances of your line.
*/
class KDEPIM_EXPORT MultiplyingLineEditor : public QWidget
{
  Q_OBJECT
  Q_PROPERTY( bool autoResizeView READ autoResizeView WRITE setAutoResizeView )
  Q_PROPERTY( bool dynamicSizeHint READ dynamicSizeHint WRITE setDynamicSizeHint )

  public:

    // We take ownership of factory
    explicit MultiplyingLineEditor( MultiplyingLineFactory* factory, QWidget *parent = 0 );

    virtual ~MultiplyingLineEditor();

    /** Get the current line factory for this instance of the widget.
     */
    MultiplyingLineFactory* factory() const;

    /** Retrieve the data from the editor */
    QList<MultiplyingLineData::Ptr> allData() const;

    /** Retrieve the data of the active line */
    MultiplyingLineData::Ptr activeData() const;

    /** Clear all lines from the widget.
     */
    void clear();

    /** Returns true if the user has made any modifications to the list of
        recipients.
    */
    bool isModified();

    /** Resets the modified flag to false.
    */
    void clearModified();

    /** Adds data to one line of the editor.
        @param data The data you want to add.
        Can be used to add an empty/default  line.
    */
    void addData( const MultiplyingLineData::Ptr &data = MultiplyingLineData::Ptr() );

    /** Removes data provided it can be found. The Data class must support operator==
        @param data The data you want to add.
    */
    void removeData( const MultiplyingLineData::Ptr &data );

    /**
      Set the width of the left most column to be the argument width.
      This method allows other widgets to align their label/combobox column with ours
      by communicating how many pixels that first column is for them.
      @param w what the left most column width should be
      @return the width that is actually being used.
      */
    int setFirstColumnWidth( int w );

    /**
      Set completion mode for all lines
      @param mode the completion mode
      */
    void setCompletionMode( KGlobalSettings::Completion mode );

    /**
     Set the underlying view's frame shape, default is none.
     @param shape of type QFrame::Shape
     */
    void setFrameStyle( int shape );

    /**
     Make the line view follow it's children's size
     @param resize turn on or off this behavior of auto resizing
     */
    void setAutoResizeView( bool resize );
    bool autoResizeView();

    /**
     * Sets whether the size hint of the editor shall be calculated
     * dynamically by the number of lines. Default is @c true.
     */
    void setDynamicSizeHint( bool dynamic );
    bool dynamicSizeHint() const;

  signals:
    void focusUp();
    void focusDown();
    void completionModeChanged( KGlobalSettings::Completion );
    void sizeHintChanged();
    void lineDeleted( int pos );
    void lineAdded( KPIM::MultiplyingLine * );

  public slots:
    void setFocus();
    void setFocusTop();
    void setFocusBottom();

  protected:
    virtual QList<MultiplyingLine*> lines() const;
    virtual MultiplyingLine *activeLine() const;
    bool mModified;

  private:
    MultiplyingLineFactory *mMultiplyingLineFactory;
    MultiplyingLineView *mView;
};
}
#endif // MULTIPLYINGLINEEDITOR_H
