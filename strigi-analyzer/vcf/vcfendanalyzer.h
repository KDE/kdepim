/*
    This file is part of KDE-PIM.

    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

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
#include <strigi/analyzerplugin.h>
#include <strigi/streamendanalyzer.h>
#include <qstring.h>
#include <kabc/vcardconverter.h>
#include "pimstrigi-analyzer_export.h"
#include "config-strigi.h"

class VcfEndAnalyzerFactory;

class PIMSTRIGI_ANALYZER_EXPORT VcfEndAnalyzer : public Strigi::StreamEndAnalyzer
{
  public:
    VcfEndAnalyzer( const VcfEndAnalyzerFactory *factory );

    const char* name() const { return "VcfEndAnalyzer"; }
    bool checkHeader( const char* header, qint32 headersize ) const;
    STRIGI_ENDANALYZER_RETVAL analyze(  Strigi::AnalysisResult& idx, Strigi::InputStream* in );

  private:
    QString formatAddress(const KABC::Address& a) const;
    const VcfEndAnalyzerFactory* m_factory;
};

class PIMSTRIGI_ANALYZER_EXPORT VcfEndAnalyzerFactory : public Strigi::StreamEndAnalyzerFactory
{
  friend class VcfEndAnalyzer;

  public:
    const Strigi::RegisteredField* givenNameField;
    const Strigi::RegisteredField* familyNameField;

    const Strigi::RegisteredField* emailField;
    const Strigi::RegisteredField* typeField;
    const Strigi::RegisteredField* homepageField;
    const Strigi::RegisteredField* commentField;

    const Strigi::RegisteredField* cellPhoneField;
    const Strigi::RegisteredField* workPhoneField;
    const Strigi::RegisteredField* homePhoneField;
    const Strigi::RegisteredField* faxPhoneField;
    const Strigi::RegisteredField* otherPhoneField;

    const Strigi::RegisteredField* photoField;

    const Strigi::RegisteredField* homeAddressField;
    const Strigi::RegisteredField* workAddressField;
    const Strigi::RegisteredField* postalAddressField;
    const Strigi::RegisteredField* otherAddressField;

    const Strigi::RegisteredField* prefixField;
    const Strigi::RegisteredField* suffixField;


    const char* name() const { return "VcfEndAnalyzer"; }
    Strigi::StreamEndAnalyzer* newInstance() const { return new VcfEndAnalyzer( this ); }
    void registerFields( Strigi::FieldRegister& );
};

class PIMSTRIGI_ANALYZER_EXPORT VcfFactoryFactory : public Strigi::AnalyzerFactoryFactory
{
  public:
    std::list<Strigi::StreamEndAnalyzerFactory*> streamEndAnalyzerFactories() const {
       std::list<Strigi::StreamEndAnalyzerFactory*> list;
       list.push_back( new VcfEndAnalyzerFactory );

       return list;
    }
};

STRIGI_ANALYZER_FACTORY(VcfFactoryFactory)

#endif
