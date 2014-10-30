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

#include "texttospeech.h"
#include "settings/pimcommonsettings.h"
#if KDEPIM_HAVE_TEXTTOSPEECH
#include <QtTextToSpeech/QTextToSpeech>
#endif

namespace PimCommon
{

class TextToSpeechPrivate
{
public:
    TextToSpeechPrivate()
        : textToSpeech(new TextToSpeech)
    {
    }

    ~TextToSpeechPrivate()
    {
        delete textToSpeech;
    }

    TextToSpeech *textToSpeech;
};

Q_GLOBAL_STATIC(TextToSpeechPrivate, sInstance)

TextToSpeech::TextToSpeech(QObject *parent)
    : QObject(parent)
#if KDEPIM_HAVE_TEXTTOSPEECH
    , mTextToSpeech(new QTextToSpeech(this))
#endif
{
    init();
}

TextToSpeech::~TextToSpeech()
{

}

void TextToSpeech::init()
{
#if KDEPIM_HAVE_TEXTTOSPEECH
    mTextToSpeech->setRate(PimCommon::PimCommonSettings::self()->rate());
    mTextToSpeech->setPitch(PimCommon::PimCommonSettings::self()->pitch());
    mTextToSpeech->setVolume(PimCommon::PimCommonSettings::self()->volume());
    mTextToSpeech->setLocale(QLocale(PimCommon::PimCommonSettings::self()->localeName()));
    connect(mTextToSpeech, &QTextToSpeech::stateChanged, this, &TextToSpeech::slotStateChanged);
#endif
}

TextToSpeech *TextToSpeech::self()
{
    return sInstance->textToSpeech; //will create it
}

void TextToSpeech::slotStateChanged()
{
#if KDEPIM_HAVE_TEXTTOSPEECH
    TextToSpeech::State state;
    switch (mTextToSpeech->state()) {
    case QTextToSpeech::Ready:
        state = TextToSpeech::Ready;
        break;
    case QTextToSpeech::Speaking:
        state = TextToSpeech::Speaking;
        break;
    case QTextToSpeech::Paused:
        state = TextToSpeech::Paused;
        break;
    case QTextToSpeech::BackendError:
        state = TextToSpeech::BackendError;
        break;
    }
    Q_EMIT stateChanged(state);
#endif
}

bool TextToSpeech::isReady() const
{
#if KDEPIM_HAVE_TEXTTOSPEECH
    return (mTextToSpeech->state() != QTextToSpeech::BackendError);
#else
    return false;
#endif
}

void TextToSpeech::say(const QString &text)
{
#if KDEPIM_HAVE_TEXTTOSPEECH
    mTextToSpeech->say(text);
#else
    Q_UNUSED(text);
#endif

}

void TextToSpeech::stop()
{
#if KDEPIM_HAVE_TEXTTOSPEECH
    mTextToSpeech->stop();
#endif
}

void TextToSpeech::pause()
{
#if KDEPIM_HAVE_TEXTTOSPEECH
    mTextToSpeech->pause();
#endif
}

void TextToSpeech::resume()
{
#if KDEPIM_HAVE_TEXTTOSPEECH
    mTextToSpeech->resume();
#endif
}

void TextToSpeech::setRate(double rate)
{
#if KDEPIM_HAVE_TEXTTOSPEECH
    mTextToSpeech->setRate(rate);
#else
    Q_UNUSED(rate);
#endif
}

void TextToSpeech::setPitch(double pitch)
{
#if KDEPIM_HAVE_TEXTTOSPEECH
    mTextToSpeech->setPitch(pitch);
#else
    Q_UNUSED(pitch);
#endif
}

void TextToSpeech::setVolume(int volume)
{
#if KDEPIM_HAVE_TEXTTOSPEECH
    mTextToSpeech->setVolume(volume);
#else
    Q_UNUSED(volume);
#endif
}

int TextToSpeech::volume() const
{
#if KDEPIM_HAVE_TEXTTOSPEECH
    return PimCommon::PimCommonSettings::self()->volume();
#else
    return 0;
#endif
}

QVector<QLocale> TextToSpeech::availableLocales() const
{
#if KDEPIM_HAVE_TEXTTOSPEECH
    return mTextToSpeech->availableLocales();
#else
    return QVector<QLocale>();
#endif
}

void TextToSpeech::setLocale(const QLocale &locale) const
{
#if KDEPIM_HAVE_TEXTTOSPEECH
    mTextToSpeech->setLocale(locale);
#else
    Q_UNUSED(locale);
#endif
}

QLocale TextToSpeech::locale() const
{
#if KDEPIM_HAVE_TEXTTOSPEECH
    return mTextToSpeech->locale();
#else
    return QLocale();
#endif
}


}
