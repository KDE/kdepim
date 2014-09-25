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

#ifndef TEXTTOSPEECHWIDGET_H
#define TEXTTOSPEECHWIDGET_H

#include <QWidget>
#include "pimcommon_export.h"
class QToolButton;
namespace PimCommon {
class TextToSpeechInterface;
class PIMCOMMON_EXPORT TextToSpeechWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TextToSpeechWidget(QWidget *parent=0);
    ~TextToSpeechWidget();

    enum State {
        Stop = 0,
        Play,
        Pause
    };
    Q_ENUMS(State)

    State state() const;
    void setState(PimCommon::TextToSpeechWidget::State state);

    void setTextToSpeechInterface(TextToSpeechInterface *interface);


Q_SIGNALS:
    void stateChanged(PimCommon::TextToSpeechWidget::State state);

private slots:
    void slotStop();
    void slotPlayPause();

private:
    void updateButtonState();
    State mState;
    QToolButton *mStopButton;
    QToolButton *mPlayPauseButton;
    TextToSpeechInterface *mTextToSpeechInterface;
};
}
#endif // TEXTTOSPEECHWIDGET_H
