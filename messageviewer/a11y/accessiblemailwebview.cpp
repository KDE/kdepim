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

#include "accessiblemailwebview.h"
#include "messageviewer/viewer/mailwebview.h"

#include <QWebFrame>

QString Q_GUI_EXPORT qTextAfterOffsetFromString(int offset, QAccessible2::BoundaryType boundaryType,
        int *startOffset, int *endOffset, const QString &text);
QString Q_GUI_EXPORT qTextBeforeOffsetFromString(int offset, QAccessible2::BoundaryType boundaryType,
        int *startOffset, int *endOffset, const QString &text);
QString Q_GUI_EXPORT qTextAtOffsetFromString(int offset, QAccessible2::BoundaryType boundaryType,
        int *startOffset, int *endOffset, const QString &text);

AccessibleMailWebView::AccessibleMailWebView(MessageViewer::MailWebView *widget):
    QAccessibleWidgetEx(widget, QAccessible::Document)
{
    m_widget = widget;
}

int AccessibleMailWebView::characterCount()
{
    return m_widget->page()->mainFrame()->toPlainText().size();
}

int AccessibleMailWebView::selectionCount()
{
    return m_widget->hasSelection() ? 1 : 0;
}

void AccessibleMailWebView::addSelection(int startOffset, int endOffset)
{
    Q_UNUSED(startOffset);
    Q_UNUSED(endOffset);
}

void AccessibleMailWebView::removeSelection(int selectionIndex)
{
    if (selectionIndex == 0) {
        m_widget->clearSelection();
    }
}

void AccessibleMailWebView::setSelection(int selectionIndex, int startOffset, int endOffset)
{
    Q_UNUSED(selectionIndex);
    Q_UNUSED(startOffset);
    Q_UNUSED(endOffset);
}

void AccessibleMailWebView::setCursorPosition(int position)
{
    Q_UNUSED(position);
}

QString AccessibleMailWebView::text(int startOffset, int endOffset)
{
    QString text = m_widget->page()->mainFrame()->toPlainText();
    text.truncate(endOffset);
    text.remove(0, startOffset);
    return text;
}

QString AccessibleMailWebView::attributes(int offset, int *startOffset, int *endOffset)
{
    Q_UNUSED(offset);
    Q_UNUSED(startOffset);
    Q_UNUSED(endOffset);
    return QString();
}

void AccessibleMailWebView::selection(int selectionIndex, int *startOffset, int *endOffset)
{
    Q_UNUSED(selectionIndex);
    *startOffset = -1;
    *endOffset = -1;
}

QRect AccessibleMailWebView::characterRect(int offset, QAccessible2::CoordinateType coordType)
{
    Q_UNUSED(offset);
    Q_UNUSED(coordType);
    return QRect();
}

int AccessibleMailWebView::offsetAtPoint(const QPoint &point, QAccessible2::CoordinateType coordType)
{
    Q_UNUSED(point);
    Q_UNUSED(coordType);
    return 0;
}

int AccessibleMailWebView::cursorPosition()
{
    return 0;
}

void AccessibleMailWebView::scrollToSubstring(int startIndex, int endIndex)
{
    Q_UNUSED(startIndex);
    Q_UNUSED(endIndex);
}

QString AccessibleMailWebView::textAfterOffset(int offset, QAccessible2::BoundaryType boundaryType, int *startOffset, int *endOffset)
{
    return qTextAfterOffsetFromString(offset, boundaryType, startOffset, endOffset, m_widget->page()->mainFrame()->toPlainText());
}

QString AccessibleMailWebView::textBeforeOffset(int offset, QAccessible2::BoundaryType boundaryType, int *startOffset, int *endOffset)
{
    return qTextBeforeOffsetFromString(offset, boundaryType, startOffset, endOffset, m_widget->page()->mainFrame()->toPlainText());
}

QString AccessibleMailWebView::textAtOffset(int offset, QAccessible2::BoundaryType boundaryType, int *startOffset, int *endOffset)
{
    return qTextAtOffsetFromString(offset, boundaryType, startOffset, endOffset, m_widget->page()->mainFrame()->toPlainText());
}
