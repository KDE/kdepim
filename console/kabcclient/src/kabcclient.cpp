//
//  Copyright (C) 2005 - 2008 Kevin Krammer <kevin.krammer@gmx.at>
//  Copyright (C) 2005 Tobias Koenig <tokoe@kde.org>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

// local includes
#include "kabcclient.h"
#include "formatfactory.h"
#include "inputformat.h"
#include "outputformat.h"

// standard includes
#include <iostream>
#include <set>
#include <string>

// Qt includes
#include <QtCore/QTextCodec>
#include <QtCore/QTimer>

// KDE includes
#include <kapplication.h>
#include <klocale.h>
#include <krandom.h>

// KABC includes
#include <kabc/addressee.h>
#include <kabc/addresseelist.h>
#include <kabc/stdaddressbook.h>

using namespace KABC;

// error messages
static const char saveError[] = I18N_NOOP("Saving modifications to address book failed");
static const char ambiguousMatch[] = I18N_NOOP("Input number %1 matches more than one contact. "
                                               "Skipping it to avoid undesired results");

///////////////////////////////////////////////////////////////////////////////

KABCClient::KABCClient(Operation operation, FormatFactory* factory)
    : QObject(0),
      m_operation(operation),
      m_formatFactory(factory),
      m_inputFormat(0),
      m_outputFormat(0),
      m_inputCodec(0),
      m_outputCodec(0),
      m_addressBook(0),
      m_inputStream(0),
      m_matchCaseSensitivity(Qt::CaseInsensitive),
      m_allowSaving(true)
{
}

///////////////////////////////////////////////////////////////////////////////

KABCClient::~KABCClient()
{
    delete m_inputFormat;
    delete m_outputFormat;
}

///////////////////////////////////////////////////////////////////////////////

bool KABCClient::setInputFormat(const QByteArray& name)
{
    switch (m_operation)
    {
        // no formatted input
        case List:
            return true;

        case Add:
        case Merge:
            if (name == "uid" || name == "search" || name == "dialog")
            {
                QString operation = QString::fromUtf8(m_operation == Add ? "add" : "merge");
                QString error = i18n("Input format '%1' not usable with operation '%2'",
                                     QString::fromLocal8Bit(name), operation);
                std::cerr << error.toLocal8Bit().data();
                std::cerr << std::endl;
                return false;
            }

        default:
            break;
    }

    m_inputFormat = m_formatFactory->inputFormat(name);

    return m_inputFormat != 0;
}

///////////////////////////////////////////////////////////////////////////////

bool KABCClient::setOutputFormat(const QByteArray& name)
{
    m_outputFormat = m_formatFactory->outputFormat(name);

    return m_outputFormat != 0;
}

///////////////////////////////////////////////////////////////////////////////

bool KABCClient::setInputOptions(const QByteArray& options)
{
    if (m_operation == List) return true;

    if (m_inputFormat == 0) return false;

    return m_inputFormat->setOptions(options);
}

///////////////////////////////////////////////////////////////////////////////

bool KABCClient::setOutputOptions(const QByteArray& options)
{
    if (m_outputFormat == 0) return false;

    return m_outputFormat->setOptions(options);
}

///////////////////////////////////////////////////////////////////////////////

bool KABCClient::setInputCodec(const QByteArray& name)
{
    if (m_operation == List) return true; // no input -> no input codec

    if (m_inputFormat == 0) return false;

    m_inputCodec = codecForName(name);

    if (m_inputCodec == 0) return false;

    return m_inputFormat->setCodec(m_inputCodec);
}

///////////////////////////////////////////////////////////////////////////////

bool KABCClient::setOutputCodec(const QByteArray& name)
{
    if (m_outputFormat == 0) return false;

    m_outputCodec = codecForName(name);

    if (m_outputCodec == 0) return false;

    return m_outputFormat->setCodec(m_outputCodec);
}

///////////////////////////////////////////////////////////////////////////////

void KABCClient::setInputStream(std::istream* stream)
{
    m_inputStream = stream;
}

///////////////////////////////////////////////////////////////////////////////

bool KABCClient::initOperation()
{
    if (m_inputStream == 0 && m_operation != List) return false;

//  Async Loading and starting operation of signal does not work
//  in case no resources can be loaded and a new standard resource is created
//  because in this case AddressBook doesn't emit signals :(
//  Workaround by loading synchronous and having the slot called by a
//  single shot timer

//     m_addressBook = KABC::StdAddressBook::self(true);
    m_addressBook = KABC::StdAddressBook::self(false);
    if (m_addressBook == 0) return false;

    KABC::StdAddressBook::setAutomaticSave(false);

//     QObject::connect(m_addressBook, SIGNAL(addressBookChanged(AddressBook*)),
//                      this, SLOT(slotAddressBookLoaded()));
    QTimer::singleShot(0, this, SLOT(slotAddressBookLoaded()));

    return true;
}

///////////////////////////////////////////////////////////////////////////////

int KABCClient::performAdd()
{
    // create a set of all currently existing UIDs
    std::set<QString> uids;
    AddressBook::ConstIterator it    = m_addressBook->constBegin();
    AddressBook::ConstIterator endIt = m_addressBook->constEnd();
    for (; it != endIt; ++it)
    {
        uids.insert((*it).uid());
    }

    bool wantSave = false;
    while (!m_inputStream->bad() && !m_inputStream->eof())
    {
        Addressee addressee = m_inputFormat->readAddressee(*m_inputStream);
        if (addressee.isEmpty()) continue;

        // make sure we really append and don't overwrite
        if (addressee.uid().isEmpty() || uids.find(addressee.uid()) != uids.end())
        {
            addressee.setUid(KRandom::randomString(10));
            uids.insert(addressee.uid());
        }

        m_addressBook->insertAddressee(addressee);
        wantSave = true;

        m_outputFormat->writeAddressee(addressee, std::cout);
        std::cout << std::endl;
    }

    if (!wantSave) return 2; // nothing added

    if (m_allowSaving)
    {
        Ticket* saveTicket = m_addressBook->requestSaveTicket();
        if (saveTicket == 0)
        {
            std::cerr << i18n(saveError).toLocal8Bit().data() << std::endl;
            return 3;
        }

        bool saved = m_addressBook->save(saveTicket);

        if (!saved)
        {
            std::cerr << i18n(saveError).toLocal8Bit().data() << std::endl;
            return 3;
        }
    }

    return (m_inputStream->bad() ? 1 : 0);
}

///////////////////////////////////////////////////////////////////////////////

int KABCClient::performRemove()
{
    bool wantSave = false;

    uint count = 0;
    while (!m_inputStream->bad() && !m_inputStream->eof())
    {
        Addressee search = m_inputFormat->readAddressee(*m_inputStream);
        if (search.isEmpty()) continue;

        count++;

        AddresseeList result;

        AddressBook::ConstIterator it    = m_addressBook->constBegin();
        AddressBook::ConstIterator endIt = m_addressBook->constEnd();
        for (; it != endIt; ++it)
        {
            if (!search.uid().isEmpty() && (*it).uid() == search.uid())
            {
                result.append(*it);
            }
            else if (!search.realName().isEmpty() &&
                     (*it).realName().indexOf(search.realName(), 0, m_matchCaseSensitivity) != -1)
            {
                result.append(*it);
            }
            else if (!search.familyName().isEmpty() &&
                     (*it).realName().indexOf(search.familyName(), 0, m_matchCaseSensitivity) != -1)
            {
                if (!search.givenName().isEmpty())
                {
                    if ((*it).realName().indexOf(search.givenName(), 0, m_matchCaseSensitivity) != -1)
                    {
                        result.append(*it);
                    }
                }
                else
                    result.append(*it);
            }
            else if (!search.givenName().isEmpty() &&
                     (*it).realName().indexOf(search.givenName(), 0, m_matchCaseSensitivity) != -1)
            {
                result.append(*it);
            }
            else if (!search.preferredEmail().isEmpty())
            {
                QStringList matches =
                    (*it).emails().filter(search.preferredEmail(), m_matchCaseSensitivity);
                if (matches.count() > 0)
                {
                    result.append(*it);
                }
            }

            if (result.count() > 1) break;
        }

        // only work with unambiguous matches
        if (result.count() == 1)
        {
            m_addressBook->removeAddressee(result[0]);
            wantSave = true;

            m_outputFormat->writeAddressee(result[0], std::cout);
            std::cout << std::endl;
        }
        else if (result.count() > 1)
        {
            std::cerr << i18n(ambiguousMatch).arg(count).toLocal8Bit().data() << std::endl;
        }
    }

    if (!wantSave) return 2; // nothing removed

    if (m_allowSaving)
    {
        Ticket* saveTicket = m_addressBook->requestSaveTicket();
        if (saveTicket == 0)
        {
            std::cerr << i18n(saveError).toLocal8Bit().data() << std::endl;
            return 3;
        }

        bool saved = m_addressBook->save(saveTicket);

        if (!saved)
        {
            std::cerr << i18n(saveError).toLocal8Bit().data() << std::endl;
            return 3;
        }
    }

    return (m_inputStream->bad() ? 1 : 0);
}

///////////////////////////////////////////////////////////////////////////////

int KABCClient::performMerge()
{
    bool wantSave = false;

    uint count = 0;
    while (!m_inputStream->bad() && !m_inputStream->eof())
    {
        Addressee addressee = m_inputFormat->readAddressee(*m_inputStream);
        if (addressee.isEmpty()) continue;

        count++;

        AddresseeList result;

        AddressBook::ConstIterator it    = m_addressBook->constBegin();
        AddressBook::ConstIterator endIt = m_addressBook->constEnd();
        for (; it != endIt; ++it)
        {
            if (!addressee.uid().isEmpty() && (*it).uid() == addressee.uid())
            {
                result.append(*it);
            }
            else if (!addressee.realName().isEmpty() &&
                     (*it).realName().indexOf(addressee.realName(), 0, m_matchCaseSensitivity) != -1)
            {
                result.append(*it);
            }
            else if (!addressee.familyName().isEmpty() &&
                     (*it).realName().indexOf(addressee.familyName(), 0, m_matchCaseSensitivity) != -1)
            {
                if (!addressee.givenName().isEmpty())
                {
                    if ((*it).realName().indexOf(addressee.givenName(),
                                              0, m_matchCaseSensitivity) != -1)
                    {
                        result.append(*it);
                    }
                }
                else
                    result.append(*it);
            }
            else if (!addressee.givenName().isEmpty() &&
                     (*it).realName().indexOf(addressee.givenName(), 0, m_matchCaseSensitivity) != -1)
            {
                result.append(*it);
            }
            else if (!addressee.preferredEmail().isEmpty())
            {
                QStringList matches =
                    (*it).emails().filter(addressee.preferredEmail(), m_matchCaseSensitivity);
                if (matches.count() > 0)
                {
                    result.append(*it);
                }
            }

            if (result.count() > 1) break;
        }

        // only work with unambiguous matches
        if (result.count() == 1)
        {
            Addressee master = result[0];
            mergeAddressees(master, addressee);

            m_addressBook->insertAddressee(master);
            wantSave = true;

            m_outputFormat->writeAddressee(master, std::cout);
            std::cout << std::endl;
        }
        else if (result.count() > 1)
        {
            std::cerr << i18n(ambiguousMatch).arg(count).toLocal8Bit().data() << std::endl;
        }
    }

    if (!wantSave) return 2; // nothing merged

    if (m_allowSaving)
    {
        Ticket* saveTicket = m_addressBook->requestSaveTicket();
        if (saveTicket == 0)
        {
            std::cerr << i18n(saveError).toLocal8Bit().data() << std::endl;
            return 3;
        }

        bool saved = m_addressBook->save(saveTicket);

        if (!saved)
        {
            std::cerr << i18n(saveError).toLocal8Bit().data() << std::endl;
            return 3;
        }
    }

    return (m_inputStream->bad() ? 1 : 0);
}

///////////////////////////////////////////////////////////////////////////////

int KABCClient::performList()
{
    AddresseeList list = m_addressBook->allAddressees();

    return m_outputFormat->writeAddresseeList(list, std::cout) ? 0 : 1;
}

///////////////////////////////////////////////////////////////////////////////

int KABCClient::performSearch()
{
    int resultValue = 2; // i.e. search didn't find any match

    AddressBook::ConstIterator endIt = m_addressBook->constEnd();

    while (!m_inputStream->bad() && !m_inputStream->eof())
    {
        Addressee search = m_inputFormat->readAddressee(*m_inputStream);
        if (search.isEmpty()) continue;

        AddresseeList result;

        AddressBook::ConstIterator it = m_addressBook->constBegin();
        for (; it != endIt; ++it)
        {
            if (!search.uid().isEmpty() && (*it).uid() == search.uid())
            {
                result.append(*it);
            }
            else if (!search.realName().isEmpty() &&
                     (*it).realName().indexOf(search.realName(), 0, m_matchCaseSensitivity) != -1)
            {
                result.append(*it);
            }
            else if (!search.familyName().isEmpty() &&
                     (*it).realName().indexOf(search.familyName(), 0, m_matchCaseSensitivity) != -1)
            {
                if (!search.givenName().isEmpty())
                {
                    if ((*it).realName().indexOf(search.givenName(), 0, m_matchCaseSensitivity) != -1)
                    {
                        result.append(*it);
                    }
                }
                else
                    result.append(*it);
            }
            else if (!search.givenName().isEmpty() &&
                     (*it).realName().indexOf(search.givenName(), 0, m_matchCaseSensitivity) != -1)
            {
                result.append(*it);
            }
            else if (!search.preferredEmail().isEmpty())
            {
                QStringList matches =
                    (*it).emails().filter(search.preferredEmail(), m_matchCaseSensitivity);
                if (matches.count() > 0)
                {
                    result.append(*it);
                }
            }
        }

        if (result.count() > 0)
        {
            resultValue = 0;

            if (!m_outputFormat->writeAddresseeList(result, std::cout))
            {
                return 1;
            }
        }
    }

    return resultValue;
}

///////////////////////////////////////////////////////////////////////////////
// taken from KAddressBook/KABCTools (c) Tobias Koenig

void KABCClient::mergePictures(KABC::Picture& master, const KABC::Picture slave)
{
    if (master.isIntern())
    {
        if (master.data().isNull())
        {
            if (slave.isIntern() && !slave.data().isNull())
                master.setData(slave.data());
            else if (!slave.isIntern() && !slave.url().isEmpty())
                master.setUrl(slave.url());
        }
    }
    else
    {
        if (master.url().isEmpty())
        {
            if (slave.isIntern() && !slave.data().isNull())
                master.setData( slave.data() );
            else if (!slave.isIntern() && !slave.url().isEmpty())
                master.setUrl(slave.url());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// derived from KAddressBook/KABCTools::mergeContacts (c) Tobias Koenig

void KABCClient::mergeAddressees(KABC::Addressee& master, const KABC::Addressee& slave)
{
    if (slave.isEmpty()) return;

    // ADR + LABEL
    const KABC::Address::List addresses = slave.addresses();
    KABC::Address::List masterAddresses = master.addresses();
    KABC::Address::List::ConstIterator addrIt ;
    for (addrIt = addresses.begin(); addrIt != addresses.end(); ++addrIt)
    {
        if (!masterAddresses.contains(*addrIt))
            master.insertAddress(*addrIt);
    }

    // BIRTHDAY
    if (master.birthday().isNull() && !slave.birthday().isNull())
        master.setBirthday(slave.birthday());

    // CATEGORIES
    const QStringList categories       = slave.categories();
    const QStringList masterCategories = master.categories();

    QStringList newCategories = masterCategories;
    QStringList::ConstIterator it;
    for (it = categories.begin(); it != categories.end(); ++it)
    {
        if (!masterCategories.contains(*it))
            newCategories.append(*it);
    }
    master.setCategories(newCategories);

    // CLASS
    if (!master.secrecy().isValid() && slave.secrecy().isValid())
        master.setSecrecy(slave.secrecy());

    // EMAIL
    const QStringList emails       = slave.emails();
    const QStringList masterEmails = master.emails();

    for (it = emails.begin(); it != emails.end(); ++it)
    {
        if (!masterEmails.contains(*it))
            master.insertEmail(*it, false);
    }

    // FN
    if (master.formattedName().isEmpty() && !slave.formattedName().isEmpty())
        master.setFormattedName(slave.formattedName());

    // GEO
    if (!master.geo().isValid() && slave.geo().isValid())
        master.setGeo(slave.geo());

    /*
    // KEY
    */
    // LOGO
    KABC::Picture logo = master.logo();
    mergePictures(logo, slave.logo());
    master.setLogo(logo);

    // MAILER
    if (master.mailer().isEmpty() && !slave.mailer().isEmpty())
        master.setMailer(slave.mailer());

    // N
    if (master.assembledName().isEmpty() && !slave.assembledName().isEmpty())
        master.setNameFromString(slave.assembledName());

    // NICKNAME
    if (master.nickName().isEmpty() && !slave.nickName().isEmpty())
        master.setNickName(slave.nickName());

    // NOTE
    if (master.note().isEmpty() && !slave.note().isEmpty())
        master.setNote(slave.note());

    // ORG
    if (master.organization().isEmpty() && !slave.organization().isEmpty())
        master.setOrganization(slave.organization());

    // PHOTO
    KABC::Picture photo = master.photo();
    mergePictures(photo, slave.photo());
    master.setPhoto( photo );

    // PROID
    if (master.productId().isEmpty() && !slave.productId().isEmpty())
        master.setProductId(slave.productId());

    // REV
    if (master.revision().isNull() && !slave.revision().isNull())
        master.setRevision(slave.revision());

    // ROLE
    if (master.role().isEmpty() && !slave.role().isEmpty())
        master.setRole(slave.role());

    // SORT-STRING
    if (master.sortString().isEmpty() && !slave.sortString().isEmpty())
        master.setSortString(slave.sortString());

    /*
    // SOUND
    */

    // TEL
    const KABC::PhoneNumber::List phones       = slave.phoneNumbers();
    const KABC::PhoneNumber::List masterPhones = master.phoneNumbers();

    KABC::PhoneNumber::List::ConstIterator phoneIt;
    for (phoneIt = phones.begin(); phoneIt != phones.end(); ++phoneIt)
    {
        if (!masterPhones.contains(*phoneIt))
            master.insertPhoneNumber(*phoneIt);
    }

    // TITLE
    if (master.title().isEmpty() && !slave.title().isEmpty())
        master.setTitle(slave.title());

    // TZ
    if (!master.timeZone().isValid() && slave.timeZone().isValid())
        master.setTimeZone(slave.timeZone());

    // ignore UID we keep the master's one

    // URL
    if (master.url().isEmpty() && !slave.url().isEmpty())
        master.setUrl(slave.url());

    // X-
    const QStringList customs       = slave.customs();
    const QStringList masterCustoms = master.customs();

    QStringList newCustoms = masterCustoms;
    for (it = customs.begin(); it != customs.end(); ++it)
    {
        if (!masterCustoms.contains(*it))
            newCustoms.append(*it);
    }
    master.setCustoms(newCustoms);
}

///////////////////////////////////////////////////////////////////////////////

QTextCodec* KABCClient::codecForName(const QByteArray& name)
{
    if (name.isEmpty()) return 0;

    if (name.toLower() == "utf-8" || name.toLower() == "utf8" || name == "utf")
    {
        return QTextCodec::codecForName("UTF-8");
    }

    if (name.toLower() == "local" || name.toLower() == "locale")
    {
        return QTextCodec::codecForLocale();
    }

    return QTextCodec::codecForName(name.toUpper());
}

///////////////////////////////////////////////////////////////////////////////

void KABCClient::slotAddressBookLoaded()
{
    // disconnect so we are not disturbed during operations
    QObject::disconnect(m_addressBook, SIGNAL(addressBookChanged(AddressBook*)),
                        this, SLOT(slotAddressBookLoaded()));

    int result = 1;

    switch (m_operation)
    {
        case Add:
            result = performAdd();
            break;

        case Remove:
            result = performRemove();
            break;

        case Merge:
            result = performMerge();
            break;

        case List:
            result = performList();
            break;

        case Search:
            result = performSearch();
            break;

        default:
            break;
    }

    KApplication::kApplication()->exit(result);
}

// End of file

