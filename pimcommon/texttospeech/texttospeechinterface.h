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


#ifndef TEXTTOSPEECHINTERFACE_H
#define TEXTTOSPEECHINTERFACE_H

#include <QObject>
#include "texttospeechwidget.h"
#include "abstracttexttospeechinterface.h"
#include "pimcommon_export.h"
namespace PimCommon {
class TextToSpeechWidget;
class PIMCOMMON_EXPORT TextToSpeechInterface : public AbstractTextToSpeechInterface
{
    Q_OBJECT
public:
    explicit TextToSpeechInterface(TextToSpeechWidget *textToSpeechWidget, QObject *parent=0);
    ~TextToSpeechInterface();

    bool isReady() const;
    void say(const QString &text);


private Q_SLOTS:
    void stateChanged(PimCommon::TextToSpeechWidget::State state);

private:
    TextToSpeechWidget *mTextToSpeechWidget;    
};
}

#endif // TEXTTOSPEECHINTERFACE_H
