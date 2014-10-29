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

#ifndef TEXTTOSPEECH_H
#define TEXTTOSPEECH_H

#include "pimcommon_export.h"
#include "config-kdepim.h"
#include <QObject>

#if KDEPIM_HAVE_TEXTTOSPEECH
class QTextToSpeech;
#endif

namespace PimCommon
{

class PIMCOMMON_EXPORT TextToSpeech : public QObject
{
    Q_OBJECT
public:
    static TextToSpeech *self();

    ~TextToSpeech();

    bool isReady() const;

    enum State {
        Ready = 0,
        Speaking,
        Paused,
        BackendError
    };

    int volume() const;
    QVector<QLocale> availableLocales() const;
    QLocale currentLocale() const;

public Q_SLOTS:
    void say(const QString &text);
    void stop();
    void pause();
    void resume();

    void setRate(double rate);
    void setPitch(double pitch);
    void setVolume(int volume);
    void setLocales(const QLocale &locale) const;

Q_SIGNALS:
    void stateChanged(TextToSpeech::State);

private Q_SLOTS:
    void slotStateChanged();

private:
    explicit TextToSpeech(QObject *parent = 0);
    friend class TextToSpeechPrivate;

    void init();
#if KDEPIM_HAVE_TEXTTOSPEECH
    QTextToSpeech *mTextToSpeech;
#endif
};
}

#endif // TEXTTOSPEECH_H
