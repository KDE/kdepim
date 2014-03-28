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

#ifndef MULTIPLYINGLINEEDITOR_P_H
#define MULTIPLYINGLINEEDITOR_P_H

#include "multiplyingline.h"
#include "multiplyinglineeditor.h"

#include <KGlobalSettings>

#include <QPointer>
#include <QScrollArea>

namespace KPIM {

class MultiplyingLineView : public QScrollArea
{
    Q_OBJECT
public:
    MultiplyingLineView( MultiplyingLineFactory* factory, MultiplyingLineEditor *parent );
    ~MultiplyingLineView(){}

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    MultiplyingLine *activeLine() const;

    MultiplyingLine *emptyLine() const;

    QList<MultiplyingLineData::Ptr> allData() const;

    /** Removes data provided it can be found. The Data class must support operator==
        @param data The data you want to remove.
    */
    void removeData( const MultiplyingLineData::Ptr &data );

    /** Returns true if the user has made any modifications to the list of
        recipients.
        @return whether the view is modified or not.
    */
    bool isModified() const;

    /** Resets the modified flag to false.
    */
    void clearModified();

    /** Activates the line */
    void activateLine( MultiplyingLine *line );

    /**QScrollArea
      * Set the width of the left most column to be the argument width.
      * This method allows other widgets to align their label/combobox column with ours
      * by communicating how many pixels that first column is for them.
      * Returns the width that is actually being used.
      */
    int setFirstColumnWidth( int );

    /**
     Make this widget follow it's children's size
     @param resize turn on or off this behavior of auto resizing
     */
    void setAutoResize( bool resize );
    bool autoResize();

    /**
     * Sets whether the size hint of the editor shall be calculated
     * dynamically by the number of lines. Default is @c true.
     */
    void setDynamicSizeHint( bool dynamic );
    bool dynamicSizeHint() const;

    QList<MultiplyingLine*> lines() const;

public slots:
    void setCompletionMode( KGlobalSettings::Completion mode );
    MultiplyingLine *addLine();

    void setFocus();
    void setFocusTop();
    void setFocusBottom();

signals:
    void focusUp();
    void focusDown();
    void focusRight();
    void completionModeChanged( KGlobalSettings::Completion );
    void sizeHintChanged();
    void lineDeleted( int pos );
    void lineAdded( KPIM::MultiplyingLine * );

protected:
    void resizeEvent( QResizeEvent * );
    void resizeView();

protected slots:
    void slotReturnPressed( KPIM::MultiplyingLine * );
    void slotDownPressed( KPIM::MultiplyingLine * );
    void slotUpPressed( KPIM::MultiplyingLine * );
    void slotDecideLineDeletion( KPIM::MultiplyingLine * );
    void slotDeleteLine();
    void moveCompletionPopup();
    void moveScrollBarToEnd();

private:
    QList<MultiplyingLine*> mLines;
    QPointer<MultiplyingLine> mCurDelLine;
    int mLineHeight;
    int mFirstColumnWidth;
    bool mModified;
    KGlobalSettings::Completion mCompletionMode;
    QWidget *mPage;
    QLayout *mTopLayout;
    MultiplyingLineFactory *mMultiplyingLineFactory;
    bool mAutoResize;
    bool mDynamicSizeHint;
};
}

#endif //MULTIPLYINGLINEEDITOR_P_H
