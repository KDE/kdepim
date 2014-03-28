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

#ifndef MULTIPLYINGLINE_H
#define MULTIPLYINGLINE_H

#include "kdepim_export.h"

#include <KGlobalSettings>

#include <QWidget>
#include <QSharedPointer>

namespace KPIM {

/**
  @short ABC representing line data
  @author Casey Link
*/
class KDEPIM_EXPORT MultiplyingLineData
{
public:
    typedef QSharedPointer<MultiplyingLineData> Ptr;
    virtual ~MultiplyingLineData(){}

    /**
      Clear data, reset to defaults
    */
    virtual void clear() = 0;
    /**
      Is the data empty?
    */
    virtual bool isEmpty() const = 0;
};

/**
 @short Abstract Base Class representing a line in the Multiplying line widget.
 This class (and its subclasses) represent the lines in the MultiplyingLineEditor. Users of the
 MultiplyingLineEditor widget should subclass this class, and add their own input widgets as members,
 then implement the pure virtual methods and connect all the appropriate slots.
 @author Casey Link
*/
class KDEPIM_EXPORT MultiplyingLine : public QWidget
{

    Q_OBJECT
public:
    explicit MultiplyingLine( QWidget *parent );
    virtual ~MultiplyingLine() {}

    /**
      This line is being activated. Focus should be set
      on the first or most important widget in the line.
      */
    virtual void activate() = 0;
    /**
     Check if whatever receives focus in activate()
     currently has focus.
     @return true if this line is active
     */
    virtual bool isActive() const = 0;

    /**
      Determine if this line was modified.
      @return true if the user has made any modifications to this
      MultiplyingLine.
    */
    virtual bool isModified() const = 0;

    /**
      Resets the modified flag to false.
    */
    virtual void clearModified() = 0;

    /**
      Retrieve the data.
      @return the data associated with this line.
    */
    virtual MultiplyingLineData::Ptr data() const = 0;
    /**
      Set the data of this line. The containing widgets should be
      populated accordingly.
      @param data the data to populate this line wit
    */
    virtual void setData( const MultiplyingLineData::Ptr &data ) = 0;

    /**
      Whether this line is empty or not. Usually there is a primary widget
      that can be tested (such as a line edit).
      @return true if this line is empty, false otherwise.
    */
    virtual bool isEmpty() const = 0;

    /**
      Set the width of the left most column to be the argument width.
      This method allows other widgets to align their label/combobox column with ours
      by communicating how many pixels that first column is for them.
      @param w the width to set the left most column to.
      @return the width that is actually being used.
    */
    virtual int setColumnWidth( int w ) = 0;

    /**
      Used to set setup the correct chain of widgets to focus on
      when the user presses tab.
      @param previous the previous widget (probably from the preceding line)

      Example with a 3 widget line:

      void YourLine::fixTabOrder( QWidget *previous ) {
        setTabOrder( previous, mLeftMost );
        setTabOrder( mLeftMost, mMiddle);
        setTabOrder( mMiddle, mRightMost);
      }
      */
    virtual void fixTabOrder( QWidget *previous ) = 0;

    /**
      @return The final widget in this line on which if the user presses
      tab focus should be given to the next line. This will commonly
      be used as the parameter of fixTabOrder( QWidget *previous ).
      @see fixTabOrder( QWidget *previous )
      */
    virtual QWidget *tabOut() const = 0;

    /**
      Clear the contents of this line. Reset to default state
     */
    virtual void clear() = 0;

    /**
      Sets the type of completion to be used for KLineEdits in this line
      @param mode the completion mode
      */
    virtual void setCompletionMode( KGlobalSettings::Completion mode ) = 0;

    /**
      If the view is resized while the completion popup is open, things might start
      to overlap. This method should hide() then show() the completionBox for any
      currently visible completion boxes.
      (somewhat hacky) Example:
      void YourLine::moveCompletionPopup() {
        if( mLineEdit->completionBox( false ) ) { // check if box is not null without creating it
          if( mLineEdit->completionBox()->isVisible() ) { // check if it is currently being shown
            mLineEdit->completionBox()->hide(); // trigger the moving
            mLineEdit->completionBox()->show();
          }
        }
      */
    virtual void moveCompletionPopup() = 0;

    /**
     * Re implement this method if you need to do something
     * before a line is deleted.
     *
     * Default implementation does nothing.
     */
    virtual void aboutToBeDeleted();

signals:
    /**
      Emitted when the return/enter key is pressed
    */
    void returnPressed( KPIM::MultiplyingLine * );
    /**
      Emitted when the down key is pressed
    */
    void downPressed( KPIM::MultiplyingLine * );
    /**
      Emitted when the up key is pressed
    */
    void upPressed( KPIM::MultiplyingLine * );
    /**
      Emitted when the right key is pressed
    */
    void rightPressed();
    /**
      Should be emitted when the line should be deleted
      */
    void deleteLine( KPIM::MultiplyingLine * );
    /**
      Emitted when the completion mode changes
    */
    void completionModeChanged( KGlobalSettings::Completion );
public slots:
    void slotPropagateDeletion();

protected slots:
    void slotReturnPressed();
    void slotFocusUp();
    void slotFocusDown();


protected:
    /**
      Handles key press events on this line.
      Default behavior handles Up and Down presses.
    */
    virtual void keyPressEvent( QKeyEvent * );
};

}

#endif // MULTIPLYINGLINE_H
