//
//  Copyright (C) 2005 - 2006 Kevin Krammer <kevin.krammer@gmx.at>
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
#include "formatfactory.h"
#include "csvtemplatefactory.h"
#include "inputformatimpls.h"
#include "outputformatimpls.h"

///////////////////////////////////////////////////////////////////////////////

FormatFactory::FormatFactory() : m_csvtemplateFactory(0)
{
    m_inputFormats.append("search");
    m_inputFormats.append("uid");
    m_inputFormats.append("vcard");
    m_inputFormats.append("email");
    m_inputFormats.append("name");
    m_inputFormats.append("csv");
    m_inputFormats.append("dialog");

    m_outputFormats.append("uid");
    m_outputFormats.append("vcard");
    m_outputFormats.append("email");
    m_outputFormats.append("mutt");
    m_outputFormats.append("csv");
}

///////////////////////////////////////////////////////////////////////////////

FormatFactory::~FormatFactory()
{
    delete m_csvtemplateFactory;
}

///////////////////////////////////////////////////////////////////////////////

InputFormat* FormatFactory::inputFormat(const QByteArray& name)
{
    if (!m_inputFormats.contains(name)) return 0;

    if (name == "search") return new SearchInput();

    if (name == "uid") return new UIDInput();

    if (name == "vcard") return new VCardInput();

    if (name == "email") return new EmailInput();

    if (name == "name") return new NameInput();

    if (name == "csv")
    {
        if (m_csvtemplateFactory == 0) m_csvtemplateFactory = new CSVTemplateFactory();

        return new CSVInput(m_csvtemplateFactory);
    }

    if (name == "dialog") return new DialogInput();

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

OutputFormat* FormatFactory::outputFormat(const QByteArray& name)
{
    if (!m_outputFormats.contains(name)) return 0;

    if (name == "uid") return new UIDOutput();

    if (name == "vcard") return new VCardOutput();

    if (name == "email") return new EmailOutput();

    if (name == "mutt") return new MuttOutput();

    if (name == "csv")
    {
        if (m_csvtemplateFactory == 0) m_csvtemplateFactory = new CSVTemplateFactory();

        return new CSVOutput(m_csvtemplateFactory);
    }

    return 0;
}

// End of file
