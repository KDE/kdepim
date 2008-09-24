/* This file is part of the KDE project
 * Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef RFC822ENDANALYZER_H
#define RFC822ENDANALYZER_H

#define STRIGI_IMPORT_API
#include <strigi/analyzerplugin.h>
#include <strigi/streamendanalyzer.h>
#include "pimstrigi-analyzer_export.h"
#include "config-strigi.h"

class Rfc822EndAnalyzerFactory;

class PIMSTRIGI_ANALYZER_EXPORT Rfc822EndAnalyzer : public Strigi::StreamEndAnalyzer
{
public:
  Rfc822EndAnalyzer( const Rfc822EndAnalyzerFactory* f );

  enum Field { From = 0, To, Subject, Date, ContentType };

  const char* name() const { return "Rfc822EndAnalyzer"; }
  bool checkHeader( const char* header, int32_t headersize ) const;
  STRIGI_ENDANALYZER_RETVAL analyze(  Strigi::AnalysisResult& idx, Strigi::InputStream* in );

private:
  const Rfc822EndAnalyzerFactory* m_factory;
};

class PIMSTRIGI_ANALYZER_EXPORT Rfc822EndAnalyzerFactory : public Strigi::StreamEndAnalyzerFactory
{
friend class Rfc822EndAnalyzer;
public:
  const Strigi::RegisteredField* field( Rfc822EndAnalyzer::Field ) const;

private:
  const Strigi::RegisteredField* fromField;
  const Strigi::RegisteredField* toField;
  const Strigi::RegisteredField* subjectField;
  const Strigi::RegisteredField* dateField;
  const Strigi::RegisteredField* contentTypeField;

  const char* name() const { return "Rfc822EndAnalyzer"; }
  Strigi::StreamEndAnalyzer* newInstance() const { return new Rfc822EndAnalyzer( this ); }
  void registerFields( Strigi::FieldRegister& );
};

class PIMSTRIGI_ANALYZER_EXPORT Rfc822FactoryFactory : public Strigi::AnalyzerFactoryFactory
{
public:
  std::list<Strigi::StreamEndAnalyzerFactory*> streamEndAnalyzerFactories() const {
     std::list<Strigi::StreamEndAnalyzerFactory*> af;
     af.push_back( new Rfc822EndAnalyzerFactory );
     return af;
  }
};

STRIGI_ANALYZER_FACTORY(Rfc822FactoryFactory)

#endif
