#ifndef FINDBARBASE_H
#define FINDBARBASE_H


/* Copyright (C) 2010 Torgny Nyblom <nyblom@kde.org>
 * Copyright (C) 2010,2011 Laurent Montel <montel@kde.org>
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

#include <QtGui/QWidget>

class QAction;
class QWebView;
class KLineEdit;
class SearchLineWidget;
class QPushButton;
class QMenu;

namespace MessageViewer
{
class FindBarBase : public QWidget
{
  Q_OBJECT

  public:
    explicit FindBarBase( QWidget * parent = 0 );
    virtual ~FindBarBase();

    QString text() const;

    void focusAndSetCursor();
    
  protected:
    virtual bool event(QEvent* e);
    virtual void clearSelections();
    virtual void searchText( bool backward, bool isAutoSearch );
    void setFoundMatch( bool match );
    void messageInfo( bool backward, bool isAutoSearch, bool found );
    QMenu *optionsMenu();
  
  public slots:
    void findNext();
    void findPrev();
    void autoSearch( const QString& str );
    void slotSearchText( bool backward = false, bool isAutoSearch = true );
  private slots:
    void caseSensitivityChanged();
    void highlightAllChanged();
    void closeBar();
    void slotClearSearch();

  protected:
    QString mLastSearchStr;
    KLineEdit * m_search;
    QAction * m_caseSensitiveAct;

    QPushButton *m_findPrevBtn;
    QPushButton *m_findNextBtn;
    QMenu *m_optionsMenu;
};

}

#endif /* FINDBARBASE_H */

