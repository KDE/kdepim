/*
    This file is part of KDE-PIM.

    Copyright (c) 2007 - 2010 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef VCFENDANALYZER_H
#define VCFENDANALYZER_H

#define STRIGI_IMPORT_API

#include "pimstrigi-analyzer_export.h"

#include <kabc/vcardconverter.h>
#include <strigi/analyzerplugin.h>
#include <strigi/streamendanalyzer.h>

#include <QtCore/QString>

class VcfEndAnalyzerFactory;

class PIMSTRIGI_ANALYZER_EXPORT VcfEndAnalyzer : public Strigi::StreamEndAnalyzer
{
  public:
    explicit VcfEndAnalyzer( const VcfEndAnalyzerFactory *factory );

    const char* name() const;
    bool checkHeader( const char* header, qint32 headersize ) const;
    STRIGI_ENDANALYZER_RETVAL analyze( Strigi::AnalysisResult &index, Strigi::InputStream *stream );

  private:
    const VcfEndAnalyzerFactory* m_factory;
};

class PIMSTRIGI_ANALYZER_EXPORT VcfEndAnalyzerFactory : public Strigi::StreamEndAnalyzerFactory
{
  friend class VcfEndAnalyzer;

  public:
    const Strigi::RegisteredField* prefixField;
    const Strigi::RegisteredField* givenNameField;
    const Strigi::RegisteredField* additionalNameField;
    const Strigi::RegisteredField* familyNameField;
    const Strigi::RegisteredField* suffixField;
    const Strigi::RegisteredField* fullnameField;
    const Strigi::RegisteredField* nicknameField;

    const Strigi::RegisteredField* emailField;
    const Strigi::RegisteredField* typeField;
    const Strigi::RegisteredField* websiteUrlField;
    const Strigi::RegisteredField* noteField;

    const Strigi::RegisteredField* phoneNumberField;

    const Strigi::RegisteredField* addressCountryField;
    const Strigi::RegisteredField* addressLocalityField;
    const Strigi::RegisteredField* addressPostOfficeBoxField;
    const Strigi::RegisteredField* addressPostalCodeField;
    const Strigi::RegisteredField* addressRegionField;
    const Strigi::RegisteredField* addressStreetField;

    const Strigi::RegisteredField* photoField;
    const Strigi::RegisteredField* uidField;
    const Strigi::RegisteredField* isPartOfField;
    const Strigi::RegisteredField* categoriesField;

    const char* name() const;
    Strigi::StreamEndAnalyzer* newInstance() const;
    void registerFields( Strigi::FieldRegister& );
};

class PIMSTRIGI_ANALYZER_EXPORT VcfFactoryFactory : public Strigi::AnalyzerFactoryFactory
{
  public:
    std::list<Strigi::StreamEndAnalyzerFactory*> streamEndAnalyzerFactories() const;
};

#ifndef Q_OS_WINCE
STRIGI_ANALYZER_FACTORY( VcfFactoryFactory )
#else
EXPORT_PLUGIN( Strigi_Plugin_Vcf,VcfFactoryFactory )
#endif

#endif
