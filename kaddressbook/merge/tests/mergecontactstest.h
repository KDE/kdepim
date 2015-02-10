/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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


#ifndef MERGECONTACTSTEST_H
#define MERGECONTACTSTEST_H

#include <QObject>

class MergeContactsTest : public QObject
{
    Q_OBJECT
public:
    MergeContactsTest();

private Q_SLOTS:
    void shouldReturnDefaultAddressWhenNoListItem();
    void shouldReturnDefaultAddressWhenOneItem();
    void noNeedManualSelectionCheckWhenEmptyList();
    void noNeedManualSelectionCheckWhenOneItem();
    void shouldMergeNotes_data();
    void shouldMergeNotes();
    void shouldMergeEmails_data();
    void shouldMergeEmails();
};

#endif // MERGECONTACTSTEST_H
