/*
    Copyright 2011  José Millán Soto <fid@gpul.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ACCESSIBLEMAILWEBVIEW_H
#define ACCESSIBLEMAILWEBVIEW_H

#include <QAccessibleTextInterface>
#include <QAccessibleWidget>

namespace MessageViewer {
class MailWebView;
}

class AccessibleMailWebView: public QAccessibleWidgetEx,
        public QAccessibleTextInterface
{
    Q_ACCESSIBLE_OBJECT
public:
    explicit AccessibleMailWebView( MessageViewer::MailWebView *widget );

    virtual int characterCount();

    virtual int selectionCount();
    virtual void addSelection( int startOffset, int endOffset );
    virtual void removeSelection( int selectionIndex );
    virtual void setSelection( int selectionIndex, int startOffset, int endOffset );
    virtual void setCursorPosition( int );

    // unhide overload QString text(Text, int) from QAccessible
    using QAccessibleWidgetEx::text;

    virtual QString text( int startOffset, int endOffset );

    virtual QString attributes( int offset, int* startOffset, int* endOffset );

    virtual void selection( int selectionIndex, int* startOffset, int* endOffset );
    virtual QRect characterRect( int offset, QAccessible2::CoordinateType coordType );
    virtual int offsetAtPoint( const QPoint& point, QAccessible2::CoordinateType coordType );
    virtual int cursorPosition();
    virtual void scrollToSubstring( int startIndex, int endIndex );
    virtual QString textAfterOffset( int offset, QAccessible2::BoundaryType boundaryType, int* startOffset, int* endOffset );
    virtual QString textBeforeOffset( int offset, QAccessible2::BoundaryType boundaryType, int* startOffset, int* endOffset );
    virtual QString textAtOffset( int offset, QAccessible2::BoundaryType boundaryType, int* startOffset, int* endOffset );
private:
    MessageViewer::MailWebView *m_widget;
};

#endif
