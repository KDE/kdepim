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
#include <KMime/Message>

class TemplateParserTester : public QObject
{
    Q_OBJECT

private slots:
    /**
     * checks whether text/plain only mails are converted to a valid HTML
     */
    void test_convertedHtml();
    void test_convertedHtml_data();

    /**
     * checks whether body element is properly extracted from a valid HTML
     */
    void test_bodyFromHtml();

    /**
     * Tests whether templates are returning required body or not
     */
    void test_processWithTemplatesForBody();
    void test_processWithTemplatesForBody_data();

    void test_processWithTemplatesForContent();
    void test_processWithTemplatesForContent_data();
};

#endif
