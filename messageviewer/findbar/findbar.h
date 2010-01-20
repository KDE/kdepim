/* This file is part of the KDE project
 *
 * Copyright (C) 2010 Torgny Nyblom <kde nyblom org>
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
#ifndef __findbar_h__
#define __findbar_h__

#include <QtGui/QWidget>
#include <QtWebKit/QWebPage>

class QAction;
class QWebView;
class KLineEdit;
class SearchLineWidget;

namespace MessageViewer
{
class FindBar : public QWidget
{
  Q_OBJECT

  public:
    explicit FindBar( QWebView * view, QWidget * parent = 0 );
    virtual ~FindBar();

    QString text() const;

    void focusAndSetCursor();

  private:
    FindBar( QWidget *parent) { Q_UNUSED(parent); }
    void clearSelections();

  public slots:
    void find();
    void findNext();
    void findPrev();
    void autoSearch( QString str );

  private slots:
    void caseSensitivityChanged();
    void highlightAllChanged();
    void closeBar();

  private:
    QWebView * m_view;
    KLineEdit * m_search;
    QAction * m_caseSensitiveAct;
    QAction * m_highlightAll;
    QWebPage::FindFlags m_searchOptions;
    QString mLastSearchStr;
};
};
#endif
