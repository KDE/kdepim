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

#ifndef TEXTTOSPEECHCONFIGINTERFACE_H
#define TEXTTOSPEECHCONFIGINTERFACE_H
#include "abstracttexttospeechconfiginterface.h"
#include "pimcommon_export.h"
namespace PimCommon
{
class PIMCOMMON_EXPORT TextToSpeechConfigInterface : public AbstractTextToSpeechConfigInterface
{
    Q_OBJECT
public:
    explicit TextToSpeechConfigInterface(QObject *parent = 0);
    ~TextToSpeechConfigInterface();

    QVector<QLocale> availableLocales() const Q_DECL_OVERRIDE;
    QLocale locale() const Q_DECL_OVERRIDE;

    void setLocale(const QLocale &locale) Q_DECL_OVERRIDE;
};
}

#endif // TEXTTOSPEECHCONFIGINTERFACE_H

