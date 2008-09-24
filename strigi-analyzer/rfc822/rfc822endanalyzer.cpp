/* This file is part of the KDE project
 * Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 * Copyright (C) 2002 Shane Wright <me@shanewright.co.uk>
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

#include "rfc822endanalyzer.h"

#include <strigi/fieldtypes.h>
#include <strigi/analysisresult.h>
#include <strigi/streamendanalyzer.h>

#include <string.h>

Rfc822EndAnalyzer::Rfc822EndAnalyzer( const Rfc822EndAnalyzerFactory* f )
      : m_factory(  f )
{
}

bool Rfc822EndAnalyzer::checkHeader(  const char* header, int32_t headersize ) const
{
    //TODO: how can we know if we have a RFC822 message here?
    //      we just return false for now anyways since we need readLine to be useful =)
    return false;
}

STRIGI_ENDANALYZER_RETVAL Rfc822EndAnalyzer::analyze( Strigi::AnalysisResult& idx, Strigi::InputStream* in )
{
    char id_from[] = "From: ";
    char id_to[] = "To: ";
    char id_subject[] = "Subject: ";
    char id_date[] = "Date: ";
    char id_contenttype[] = "Content-Type: ";

    // we need a buffer for lines
    char linebuf[4096];
    linebuf[0] = 0;

    bool foundFrom = false;
    bool foundTo = false;
    bool foundDate = false;
    bool foundSubject = false;
    bool foundContentType = false;

    while ( in->status() == Strigi::Ok ) {
        //TODO: reenable when readline is implemented in StreamBase
        //in->readLine(linebuf, sizeof( linebuf ) );

        if (!foundFrom && memcmp(linebuf, id_from, 6) == 0) {
            idx.addValue( m_factory->field( From ), linebuf + 6 );
            foundFrom = true;
        } else if (!foundTo && memcmp(linebuf, id_to, 4) == 0) {
            idx.addValue( m_factory->field( To ), linebuf + 4 );
            foundTo = true;
        } else if (!foundSubject && memcmp(linebuf, id_subject, 9) == 0) {
            idx.addValue( m_factory->field( Subject ), linebuf + 9 );
            foundSubject = true;
        } else if (!foundDate && memcmp(linebuf, id_date, 6) == 0) {
            idx.addValue( m_factory->field( Date ), linebuf + 6 );
            foundDate = true;
        } else if (!foundContentType &&
                   memcmp(linebuf, id_contenttype, 14) == 0) {
            idx.addValue( m_factory->field( ContentType ), linebuf + 14 );
            foundContentType = true;
        }

        if ( foundFrom && foundTo && foundDate &&
             foundSubject && foundContentType ) {
            return Strigi::Ok;
        }
    };

    return Strigi::Error;
}

const Strigi::RegisteredField* Rfc822EndAnalyzerFactory::field( Rfc822EndAnalyzer::Field f ) const
{
  switch ( f ) {
    case Rfc822EndAnalyzer::From:
      return fromField;
      break;
    case Rfc822EndAnalyzer::To:
      return toField;
      break;
    case Rfc822EndAnalyzer::Subject:
      return subjectField;
      break;
    case Rfc822EndAnalyzer::Date:
      return dateField;
      break;
    case Rfc822EndAnalyzer::ContentType:
    default:
      return contentTypeField;
      break;
  }
}

void Rfc822EndAnalyzerFactory::registerFields( Strigi::FieldRegister& reg )
{
  fromField = reg.registerField("email.from", Strigi::FieldRegister::stringType, 1, 0 );
  toField = reg.registerField("email.to", Strigi::FieldRegister::stringType, 1, 0 );
  subjectField = reg.registerField("message.subject", Strigi::FieldRegister::stringType, 1, 0 );
  dateField = reg.registerField("content.creation_time", Strigi::FieldRegister::stringType, 1, 0 );
  contentTypeField = reg.registerField("email.content_type", Strigi::FieldRegister::stringType, 1, 0 );
}

