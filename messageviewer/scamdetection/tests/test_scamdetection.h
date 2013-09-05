/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#ifndef TEST_SCAMDETECTION_H
#define TEST_SCAMDETECTION_H


#include <qtest_kde.h>
#include <QObject>
class QWebFrame;
class ScamDetectionTest : public QObject
{
    Q_OBJECT
private slots:
    void testIp();
    void testNoScam();
    void testHref();

private:
    bool scanPage(QWebFrame *frame);
    bool testHtml(const QString &content);
};

#endif
