/*
  Copyright (c) 2012-2012 Montel Laurent <montel@kde.org>
  
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

#ifndef ABSTRACTTRANSLATOR_H
#define ABSTRACTTRANSLATOR_H

#include <QString>
#include <QMap>
#include <QObject>

class KComboBox;

namespace PimCommon {
class AbstractTranslator : public QObject
{
    Q_OBJECT
public:
    explicit AbstractTranslator();
    virtual ~AbstractTranslator();

    virtual void translate() = 0;

    virtual QMap<QString, QMap<QString, QString> > initListLanguage(KComboBox* from) = 0;

    QString resultTranslate() const;
    void setInputText(const QString& text);
    void setFrom(const QString& language);
    void setTo(const QString& language);

Q_SIGNALS:
    void translateDone();
    void translateFailed(bool result);

protected:
    QString mInputText;
    QString mFrom;
    QString mTo;
    QString mResult;
};
}

#endif // ABSTRACTTRANSLATOR_H
