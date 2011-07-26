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

#ifndef KRSS_TAGPROVIDERTEST_H
#define KRSS_TAGPROVIDERTEST_H

#include "tag.h"

#include <QtCore/QObject>
#include <boost/shared_ptr.hpp>

namespace KRss {
class TagProvider;
}

class TagProviderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void createTagTest();
    void createExistingTag();
    void modifyTagTest();
    void deleteEmptyTagTest();
    //void deleteNonEmptyTagTest();

private:
    boost::shared_ptr<KRss::TagProvider> m_tagProvider;
    KRss::Tag m_tag;
};

#endif // KRSS_TAGPROVIDERTEST_H
