//
//  Copyright (C) 2005 Kevin Krammer <kevin.krammer@gmx.at>
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

#ifndef OUTPUTFORMATIMPLS_H
#define OUTPUTFORMATIMPLS_H

// local includes
#include "outputformat.h"

// forward declarations
class CSVTemplate;
class CSVTemplateFactory;;
class QTextCodec;

namespace KABC
{
    class VCardConverter;
};

class UIDOutput : public OutputFormat
{
public:
    virtual QString description() const;
    
    virtual bool setOptions(const QCString& options);
    
    virtual bool setCodec(QTextCodec* codec);
    
    virtual bool writeAddressee(const KABC::Addressee& addressee, std::ostream& stream);
    virtual bool writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                    std::ostream& stream);

private:
    QTextCodec* m_codec;
};

class VCardOutput : public OutputFormat
{
public:
    VCardOutput();
    virtual ~VCardOutput();
    
    virtual QString description() const;
    
    virtual bool setOptions(const QCString& options);
    virtual QString optionUsage() const;

    virtual bool setCodec(QTextCodec* codec);
    
    virtual bool writeAddressee(const KABC::Addressee& addressee, std::ostream& stream);
    virtual bool writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                    std::ostream& stream);

private:
    KABC::VCardConverter* m_converter;
    int m_vCardVersion;
    QTextCodec* m_codec;
};

class EmailOutput : public OutputFormat
{
public:
    EmailOutput();

    virtual QString description() const;
    
    virtual bool setOptions(const QCString& options);
    virtual QString optionUsage() const;
    
    virtual bool setCodec(QTextCodec* codec);
    
    virtual bool writeAddressee(const KABC::Addressee& addressee, std::ostream& stream);
    virtual bool writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                    std::ostream& stream);

private:
    bool m_allEmails;
    bool m_includeName;
    QTextCodec* m_codec;

private:
    QString decorateEmail(const KABC::Addressee& addressee, const QString& email) const;
};

class MuttOutput : public OutputFormat
{
public:
    MuttOutput();

    virtual QString description() const;
    
    virtual bool setOptions(const QCString& options);
    virtual QString optionUsage() const;
    
    virtual bool setCodec(QTextCodec* codec);
    
    virtual bool writeAddressee(const KABC::Addressee& addressee, std::ostream& stream);
    virtual bool writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                    std::ostream& stream);

private:
    bool m_allEmails;
    bool m_queryFormat;
    bool m_altKeyFormat;
    QTextCodec* m_codec;

private:
    QString key(const KABC::Addressee& addressee) const;
};

class CSVOutput : public OutputFormat
{
public:
    CSVOutput(CSVTemplateFactory* templateFactory);
    virtual ~CSVOutput();

    virtual QString description() const;
    
    virtual bool setOptions(const QCString& options);
    virtual QString optionUsage() const;
    
    virtual bool setCodec(QTextCodec* codec);
    
    virtual bool writeAddressee(const KABC::Addressee& addressee, std::ostream& stream);
    virtual bool writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                    std::ostream& stream);

private:
    QTextCodec*  m_codec;
    CSVTemplate* m_template;
    CSVTemplateFactory* m_templateFactory;
};

#endif

// End of file
