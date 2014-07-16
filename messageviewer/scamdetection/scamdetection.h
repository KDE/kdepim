/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef SCAMDETECTION_H
#define SCAMDETECTION_H

#include "messageviewer/messageviewer_export.h"
#include <QObject>
#include <QPointer>

class QWebElement;
class QWebFrame;

namespace MessageViewer {
class ScamDetectionDetailsDialog;
class ScamCheckShortUrl;
class MESSAGEVIEWER_EXPORT ScamDetection : public QObject
{
    Q_OBJECT
public:
    explicit ScamDetection(QObject *parent = 0);
    ~ScamDetection();

    void scanPage(QWebFrame *frame);

    ScamCheckShortUrl *scamCheckShortUrl() const;

    static bool scanFrame(const QWebElement &rootElement, QString &details);

public Q_SLOTS:
    void showDetails();

Q_SIGNALS:
    void messageMayBeAScam();

private:
    QString mDetails;
    QPointer<MessageViewer::ScamDetectionDetailsDialog> mDetailsDialog;
    ScamCheckShortUrl *mCheckShortUrl;
};
}

#endif // SCAMDETECTION_H
