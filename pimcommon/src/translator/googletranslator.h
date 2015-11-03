/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#ifndef GOOGLETRANSLATOR_H
#define GOOGLETRANSLATOR_H

#include <QObject>
#include <QNetworkReply>
class QNetworkAccessManager;
class KComboBox;
class QNetworkReply;

namespace PimCommon
{
class GoogleTranslator : public QObject
{
    Q_OBJECT
public:
    explicit GoogleTranslator(QObject *parent = Q_NULLPTR);
    ~GoogleTranslator();

    QMap<QString, QMap<QString, QString> > initListLanguage(KComboBox *from);
    void translate();
    void debug();
    void clear();

    QString resultTranslate() const;
    void setInputText(const QString &text);
    void setFrom(const QString &language);
    void setTo(const QString &language);

private Q_SLOTS:
    void slotTranslateFinished(QNetworkReply *);
    void slotError(QNetworkReply::NetworkError /*error*/);

Q_SIGNALS:
    void translateDone();
    void translateFailed(bool result, const QString &errorMessage = QString());

private:
    QString mInputText;
    QString mFrom;
    QString mTo;
    QString mResult;
    QString mJsonData;
    QNetworkAccessManager *mNetworkAccessManager;
};
}

#endif // GOOGLETRANSLATOR_H
