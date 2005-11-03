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

#ifndef INPUTFORMATIMPLS_H
#define INPUTFORMATIMPLS_H

// local includes
#include "inputformat.h"

// forward declarations
class CSVTemplate;
class CSVTemplateFactory;
class DialogInputPrivate;

namespace KABC
{
    class VCardConverter;
};

class UIDInput : public InputFormat
{
public:
    virtual QString description() const;
    
    virtual bool setOptions(const QCString& options);
    
    virtual bool setCodec(QTextCodec* codec);
    
    virtual KABC::Addressee readAddressee(std::istream& stream);

private:
    QTextCodec* m_codec;
};

class VCardInput : public InputFormat
{
public:
    VCardInput();
    virtual ~VCardInput();
    
    virtual QString description() const;
    
    virtual bool setOptions(const QCString& options);

    virtual bool setCodec(QTextCodec* codec);
    
    virtual KABC::Addressee readAddressee(std::istream& stream);

private:
    KABC::VCardConverter* m_converter;
    QTextCodec* m_codec;
};

class EmailInput : public InputFormat
{
public:
    virtual QString description() const;
    
    virtual bool setOptions(const QCString& options);
    
    virtual bool setCodec(QTextCodec* codec);
    
    virtual KABC::Addressee readAddressee(std::istream& stream);

private:
    QTextCodec* m_codec;
};

class SearchInput : public InputFormat
{
public:
    virtual QString description() const;
    
    virtual bool setOptions(const QCString& options);
    
    virtual bool setCodec(QTextCodec* codec);
    
    virtual KABC::Addressee readAddressee(std::istream& stream);

private:
    QTextCodec* m_codec;
};

class NameInput : public InputFormat
{
public:
    virtual QString description() const;
    
    virtual bool setOptions(const QCString& options);
    
    virtual bool setCodec(QTextCodec* codec);
    
    virtual KABC::Addressee readAddressee(std::istream& stream);

private:
    QTextCodec* m_codec;
};

class CSVInput : public InputFormat
{
public:
    CSVInput(CSVTemplateFactory* templateFactory);
    
    virtual QString description() const;
    
    virtual bool setOptions(const QCString& options);
    virtual QString optionUsage() const;
    
    virtual bool setCodec(QTextCodec* codec);
    
    virtual KABC::Addressee readAddressee(std::istream& stream);

private:
    QTextCodec*  m_codec;
    CSVTemplate* m_template;
    CSVTemplateFactory* m_templateFactory;

private:
    QStringList split(const QString& values) const;
};

class DialogInput : public InputFormat
{
public:
    DialogInput();
    virtual ~DialogInput();

    virtual QString description() const;
    
    virtual bool setOptions(const QCString& options);
    
    virtual bool setCodec(QTextCodec* codec);
    
    virtual KABC::Addressee readAddressee(std::istream& stream);

private:
    DialogInputPrivate* m_private;
};

#endif

// End of file
