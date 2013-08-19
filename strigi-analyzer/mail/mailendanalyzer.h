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

#ifndef MAILENDANALYZER_H
#define MAILENDANALYZER_H

#define STRIGI_IMPORT_API

#include <config-strigi.h>
#include "pimstrigi-analyzer_export.h"

#include <strigi/analyzerplugin.h>
#include <strigi/streamendanalyzer.h>

#include <kcomponentdata.h>

#include <QtCore/QString>

class QCoreApplication;

namespace StrigiEndAnalyzer {

class MailEndAnalyzerFactory;

class PIMSTRIGI_ANALYZER_EXPORT MailEndAnalyzer : public Strigi::StreamEndAnalyzer
{
  public:
    enum Field {
      SubjectField,
      FromField,
      SenderField,
      ToField,
      CcField,
      BccField,
      MessageIdField,
      ReferencesField,
      InReplyToField,
      ContentTypeField,
      MessageContentField,
      SentDateField,
      TypeField
    };

    explicit MailEndAnalyzer( const MailEndAnalyzerFactory *factory );
    ~MailEndAnalyzer();

    const char* name() const;
    bool checkHeader( const char* header, qint32 headersize ) const;
    STRIGI_ENDANALYZER_RETVAL analyze( Strigi::AnalysisResult &index, Strigi::InputStream *stream );

    void addValue( Field field, const QString &value );

  private:
    const MailEndAnalyzerFactory* m_factory;
    Strigi::AnalysisResult *m_index;
    QCoreApplication *m_coreApplication;
};

class PIMSTRIGI_ANALYZER_EXPORT MailEndAnalyzerFactory : public Strigi::StreamEndAnalyzerFactory
{
  friend class MailEndAnalyzer;

  public:
    const Strigi::RegisteredField* subjectField;
    const Strigi::RegisteredField* fromField;
    const Strigi::RegisteredField* senderField;
    const Strigi::RegisteredField* toField;
    const Strigi::RegisteredField* ccField;
    const Strigi::RegisteredField* bccField;
    const Strigi::RegisteredField* messageIdField;
    const Strigi::RegisteredField* referencesField;
    const Strigi::RegisteredField* inReplyToField;
    const Strigi::RegisteredField* contentTypeField;
    const Strigi::RegisteredField* messageContentField;
    const Strigi::RegisteredField* sentDateField;

    const Strigi::RegisteredField* typeField;
    const Strigi::RegisteredField* isPartOfField;

    const char* name() const;
    Strigi::StreamEndAnalyzer* newInstance() const;
    void registerFields( Strigi::FieldRegister& );
};

class PIMSTRIGI_ANALYZER_EXPORT MailFactoryFactory : public Strigi::AnalyzerFactoryFactory
{
  public:
    MailFactoryFactory();

    std::list<Strigi::StreamEndAnalyzerFactory*> streamEndAnalyzerFactories() const;

  private:
    KComponentData componentData;
};

}

#endif
