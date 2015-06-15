/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef TEXTTOSPEECHACTIONS_H
#define TEXTTOSPEECHACTIONS_H

#include <QObject>
#include "pimcommon_export.h"
#include "texttospeechwidget.h"
class QAction;
namespace PimCommon
{
class PIMCOMMON_EXPORT TextToSpeechActions : public QObject
{
    Q_OBJECT
public:
    explicit TextToSpeechActions(QObject *parent = Q_NULLPTR);
    ~TextToSpeechActions();

    QAction *stopAction() const;

    QAction *playPauseAction() const;

    TextToSpeechWidget::State state() const;
    void setState(const TextToSpeechWidget::State &state);

Q_SIGNALS:
    void stateChanged(PimCommon::TextToSpeechWidget::State state);

private Q_SLOTS:
    void slotStop();
    void slotPlayPause();

private:
    void updateButtonState();
    TextToSpeechWidget::State mState;
    QAction *mStopAction;
    QAction *mPlayPauseAction;
};
}

#endif // TEXTTOSPEECHACTIONS_H
