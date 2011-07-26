/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KRSSRESOURCE_IMPORTOPMLJOBTEST_H
#define KRSSRESOURCE_IMPORTOPMLJOBTEST_H

#include <Akonadi/Collection>
#include <QtCore/QObject>
#include <boost/shared_ptr.hpp>

namespace KRss {
class TagProvider;
}

class ImportOpmlJobTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testGoodOpml();
    void testImportDuplicates();
    void testBrokenOpml();

private:
    boost::shared_ptr<const KRss::TagProvider> m_tagProvider;
    Akonadi::Collection::Id m_freedesktopId;
};

#endif // KRSSRESOURCE_IMPORTOPMLJOBTEST_H
