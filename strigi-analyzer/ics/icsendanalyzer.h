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

#ifndef ICSENDANALYZER_H
#define ICSENDANALYZER_H

#define STRIGI_IMPORT_API
#include <strigi/analyzerplugin.h>
#include <strigi/streamendanalyzer.h>
#include <kcomponentdata.h>
#include "pimstrigi-analyzer_export.h"
#include "config-strigi.h"

class IcsEndAnalyzerFactory;

class PIMSTRIGI_ANALYZER_EXPORT IcsEndAnalyzer : public Strigi::StreamEndAnalyzer
{
public:
  IcsEndAnalyzer( const IcsEndAnalyzerFactory* f );

  enum Field { ProductId = 0, Events, Journals, Todos, TodosCompleted, TodosOverdue };

  const char* name() const { return "IcsEndAnalyzer"; }
  bool checkHeader( const char* header, qint32 headersize ) const;
  STRIGI_ENDANALYZER_RETVAL analyze(  Strigi::AnalysisResult& idx, Strigi::InputStream* in );

private:
  const IcsEndAnalyzerFactory* m_factory;
};

class PIMSTRIGI_ANALYZER_EXPORT IcsEndAnalyzerFactory : public Strigi::StreamEndAnalyzerFactory
{
friend class IcsEndAnalyzer;
public:
  const Strigi::RegisteredField* field( IcsEndAnalyzer::Field ) const;

private:
  const Strigi::RegisteredField* productIdField;
  const Strigi::RegisteredField* eventsField;
  const Strigi::RegisteredField* journalsField;
  const Strigi::RegisteredField* todosField;
  const Strigi::RegisteredField* todosCompletedField;
  const Strigi::RegisteredField* todosOverdueField;

  const char* name() const { return "IcsEndAnalyzer"; }
  Strigi::StreamEndAnalyzer* newInstance() const { return new IcsEndAnalyzer( this ); }
  void registerFields( Strigi::FieldRegister& );
};

class PIMSTRIGI_ANALYZER_EXPORT IcsFactoryFactory : public Strigi::AnalyzerFactoryFactory
{
private:
  KComponentData kcomponentdata;
public:
  IcsFactoryFactory() :kcomponentdata("IcsFactoryFactory") {}
  std::list<Strigi::StreamEndAnalyzerFactory*> streamEndAnalyzerFactories() const {
     if (!kcomponentdata.isValid()) {
         qFatal("KComponentData is not valid.");
     }
     std::list<Strigi::StreamEndAnalyzerFactory*> af;
     af.push_back( new IcsEndAnalyzerFactory );
     return af;
  }
};

STRIGI_ANALYZER_FACTORY(IcsFactoryFactory)

#endif
