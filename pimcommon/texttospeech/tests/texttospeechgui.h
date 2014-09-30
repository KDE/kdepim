/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef TEXTTOSPEECHGUI_H
#define TEXTTOSPEECHGUI_H

#include <QMainWindow>
class QTextEdit;
class TextToSpeechGui : public QMainWindow
{
    Q_OBJECT
public:
    explicit TextToSpeechGui(QWidget *parent = 0);
    ~TextToSpeechGui();

private Q_SLOTS:
    void slotTextToSpeech();

private:
    QTextEdit *mEdit;
};

#endif // TEXTTOSPEECHGUI_H
