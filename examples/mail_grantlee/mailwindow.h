/*
  Example of theming using Grantlee.

  Copyright (c) 2010 Ronny Yabar Aizcorbe <ronnycontacto@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License version 3 only, as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License version 3 for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef MAILWINDOW_H
#define MAILWINDOW_H

#include <QtGui/QMainWindow>
#include <QSplitter>
#include <QWebView>
#include <QComboBox>
#include <QLabel>

namespace Grantlee
{
    class Engine;
}

class MailWindow : public QMainWindow
{
    Q_OBJECT

public:
    MailWindow(QWidget *parent = 0);
    ~MailWindow();

private Q_SLOTS:
    void renderMail( const QString &themeName );

private:
    QSplitter *splitter;
    QWebView *webView;
    QComboBox *comboThemes;
    QLabel *label;

    Grantlee::Engine *engine;
};

#endif // MAILWINDOW_H
