/*
    This file is part of KDE-PIM.

    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
       a KDAB Group company, info@kdab.net,
       author Tobias Koenig <tokoe@kde.org>

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

#ifndef CTGENDANALYZER_H
#define CTGENDANALYZER_H

#define STRIGI_IMPORT_API

#include "pimstrigi-analyzer_export.h"

#include <strigi/analyzerplugin.h>
#include <strigi/streamendanalyzer.h>

#include <QtCore/QString>

class CtgEndAnalyzerFactory;

class PIMSTRIGI_ANALYZER_EXPORT CtgEndAnalyzer : public Strigi::StreamEndAnalyzer
{
  public:
    explicit CtgEndAnalyzer( const CtgEndAnalyzerFactory *factory );

    const char* name() const;
    bool checkHeader( const char* header, qint32 headersize ) const;
    STRIGI_ENDANALYZER_RETVAL analyze( Strigi::AnalysisResult &index, Strigi::InputStream *stream );

  private:
    const CtgEndAnalyzerFactory* m_factory;
};

class PIMSTRIGI_ANALYZER_EXPORT CtgEndAnalyzerFactory : public Strigi::StreamEndAnalyzerFactory
{
  friend class CtgEndAnalyzer;

  public:
    const Strigi::RegisteredField* nameField;
    const Strigi::RegisteredField* typeField;
    const Strigi::RegisteredField* isPartOfField;

    const char* name() const;
    Strigi::StreamEndAnalyzer* newInstance() const;
    void registerFields( Strigi::FieldRegister& );
};

class PIMSTRIGI_ANALYZER_EXPORT CtgFactoryFactory : public Strigi::AnalyzerFactoryFactory
{
  public:
    std::list<Strigi::StreamEndAnalyzerFactory*> streamEndAnalyzerFactories() const;
};

#ifndef Q_OS_WINCE
STRIGI_ANALYZER_FACTORY( CtgFactoryFactory )
#else
EXPORT_PLUGIN( Strigi_Plugin_Ctg,CtgFactoryFactory )
#endif

#endif
