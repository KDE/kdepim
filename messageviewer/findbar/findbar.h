/* Copyright (C) 2010 Torgny Nyblom <kde@nyblom.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef MESSAGEVIEWER_FINDBAR_FINDBAR_H
#define MESSAGEVIEWER_FINDBAR_FINDBAR_H

#include <QtGui/QWidget>

class QAction;
class QWebView;
class KLineEdit;
class SearchLineWidget;
class QPushButton;

#ifdef KDEPIM_NO_WEBKIT
# define MESSAGEVIEWER_FINDBAR_NO_HIGHLIGHT_ALL
#endif

namespace MessageViewer {
  class MailWebView;
}

namespace MessageViewer
{
class FindBar : public QWidget
{
  Q_OBJECT

  public:
    explicit FindBar( MailWebView * view, QWidget * parent = 0 );
    virtual ~FindBar();

    QString text() const;

    void focusAndSetCursor();

  protected:
    virtual bool event(QEvent* e);


  private:
    FindBar( QWidget *parent) { Q_UNUSED(parent); }
    void clearSelections();

    void setFoundMatch( bool match );

  public slots:
    void findNext();
    void findPrev();
    void autoSearch( const QString& str );
    void searchText( bool backward = false, bool isAutoSearch = true );
  private slots:
    void caseSensitivityChanged();
    void highlightAllChanged();
    void closeBar();
    void slotClearSearch();
  private:
    MailWebView * m_view;
    KLineEdit * m_search;
    QAction * m_caseSensitiveAct;
#ifndef MESSAGEVIEWER_FINDBAR_NO_HIGHLIGHT_ALL
    QAction * m_highlightAll;
#endif
    QString mLastSearchStr;
    QPushButton *m_findPrevBtn;
    QPushButton *m_findNextBtn;
};

}

#endif
