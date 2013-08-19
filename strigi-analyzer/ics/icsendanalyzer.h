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

#include "pimstrigi-analyzer_export.h"

#include <kcalcore/incidence.h>
#include <kcomponentdata.h>

#include <strigi/analyzerplugin.h>
#include <strigi/streamendanalyzer.h>

class IcsEndAnalyzerFactory;

class PIMSTRIGI_ANALYZER_EXPORT IcsEndAnalyzer : public Strigi::StreamEndAnalyzer
{
  public:
    explicit IcsEndAnalyzer( const IcsEndAnalyzerFactory *factory );

    const char* name() const;
    bool checkHeader( const char* header, qint32 headersize ) const;
    STRIGI_ENDANALYZER_RETVAL analyze( Strigi::AnalysisResult &index, Strigi::InputStream *stream );

  private:
    void addIncidenceValues( Strigi::AnalysisResult &index, const KCalCore::Incidence::Ptr &incidence );

    const IcsEndAnalyzerFactory* m_factory;
};

class PIMSTRIGI_ANALYZER_EXPORT IcsEndAnalyzerFactory : public Strigi::StreamEndAnalyzerFactory
{
  friend class IcsEndAnalyzer;

  private:
    const Strigi::RegisteredField* productIdField;
    const Strigi::RegisteredField* eventsField;
    const Strigi::RegisteredField* journalsField;
    const Strigi::RegisteredField* todosField;
    const Strigi::RegisteredField* todosCompletedField;
    const Strigi::RegisteredField* todosOverdueField;
    const Strigi::RegisteredField* typeField;
    const Strigi::RegisteredField* uidField;
    const Strigi::RegisteredField* categoryField;
    const Strigi::RegisteredField* descriptionField;
    const Strigi::RegisteredField* dtStartField;
    const Strigi::RegisteredField* dtEndField;
    const Strigi::RegisteredField* dtDueField;
    const Strigi::RegisteredField* locationField;
    const Strigi::RegisteredField* summaryField;
    const Strigi::RegisteredField* isPartOfField;

    const char* name() const;
    Strigi::StreamEndAnalyzer* newInstance() const;
    void registerFields( Strigi::FieldRegister& );
};

class PIMSTRIGI_ANALYZER_EXPORT IcsFactoryFactory : public Strigi::AnalyzerFactoryFactory
{
  public:
    IcsFactoryFactory();

    std::list<Strigi::StreamEndAnalyzerFactory*> streamEndAnalyzerFactories() const;

  private:
    KComponentData componentData;
};

#ifndef Q_OS_WINCE
STRIGI_ANALYZER_FACTORY( IcsFactoryFactory )
#else
EXPORT_PLUGIN( Strigi_Plugin_Ics,IcsFactoryFactory )
#endif

#endif
