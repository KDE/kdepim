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

namespace MessageViewer
{
class MailWebView;
}

class AccessibleMailWebView: public QAccessibleWidget, public QAccessibleTextInterface
{
public:
    explicit AccessibleMailWebView(MessageViewer::MailWebView *widget);

    virtual int characterCount() const;

    virtual int selectionCount() const;
    virtual void addSelection(int startOffset, int endOffset);
    virtual void removeSelection(int selectionIndex);
    virtual void setSelection(int selectionIndex, int startOffset, int endOffset);
    virtual void setCursorPosition(int);

    virtual QString text(int startOffset, int endOffset) const;

    virtual QString attributes(int offset, int *startOffset, int *endOffset) const;

    virtual void selection(int selectionIndex, int *startOffset, int *endOffset) const;
    virtual QRect characterRect(int offset) const;
    virtual int offsetAtPoint(const QPoint &point) const;
    virtual int cursorPosition() const;
    virtual void scrollToSubstring(int startIndex, int endIndex);
private:
    MessageViewer::MailWebView *m_widget;
};

#endif
