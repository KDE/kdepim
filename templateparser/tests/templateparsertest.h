/* Copyright 2011 Sudhendu Kumar <sudhendu.kumar.roy@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef TEMPLATEPARSERTEST_H
#define TEMPLATEPARSERTEST_H


#include <QObject>

class TemplateParserTester : public QObject
{
  Q_OBJECT

  private slots:
    void test_plainMessageText();
    void test_htmlMessageText();
    void test_quotedPlainText();
    void test_quotedHtmlText();
    void test_plainToHtml();
    void test_makeValidHtml();
    void test_createMultipartAlternative();
    void test_createPlainPart();
    void test_addProcessedBodyToMessage();
    void test_processWithTemplate();
};

#endif
