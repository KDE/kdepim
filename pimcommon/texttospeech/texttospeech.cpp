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
#ifdef KDEPIM_HAVE_TEXTTOSPEECH
#include <QtTextToSpeech/QTextToSpeech>
#endif

namespace PimCommon {

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
#ifdef KDEPIM_HAVE_TEXTTOSPEECH
    ,mTextToSpeech(new QTextToSpeech(this))
#endif
{
}

TextToSpeech::~TextToSpeech()
{

}

TextToSpeech *TextToSpeech::self()
{
    return sInstance->textToSpeech; //will create it
}

bool TextToSpeech::isReady() const
{
#ifdef KDEPIM_HAVE_TEXTTOSPEECH
    return (mTextToSpeech->state() != QTextToSpeech::BackendError);
#else
    return false;
#endif
}

void TextToSpeech::say(const QString &text)
{
#ifdef KDEPIM_HAVE_TEXTTOSPEECH
    mTextToSpeech->say(text);
#else
    Q_UNUSED(text);
#endif

}

void TextToSpeech::stop()
{
#ifdef KDEPIM_HAVE_TEXTTOSPEECH
    mTextToSpeech->stop();
#endif
}

void TextToSpeech::pause()
{
#ifdef KDEPIM_HAVE_TEXTTOSPEECH
    mTextToSpeech->pause();
#endif
}

void TextToSpeech::resume()
{
#ifdef KDEPIM_HAVE_TEXTTOSPEECH
    mTextToSpeech->resume();
#endif
}

void TextToSpeech::setRate(double rate)
{
#ifdef KDEPIM_HAVE_TEXTTOSPEECH
    mTextToSpeech->setRate(rate);
#else
    Q_UNUSED(rate);
#endif
}

void TextToSpeech::setPitch(double pitch)
{
#ifdef KDEPIM_HAVE_TEXTTOSPEECH
    mTextToSpeech->setPitch(pitch);
#else
    Q_UNUSED(pitch);
#endif
}

void TextToSpeech::setVolume(double volume)
{
#ifdef KDEPIM_HAVE_TEXTTOSPEECH
    mTextToSpeech->setVolume(volume);
#else
    Q_UNUSED(volume);
#endif
}
}
