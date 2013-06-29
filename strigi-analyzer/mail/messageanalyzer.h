/*
    Copyright (c) 2006, 2009 Volker Krause <vkrause@kde.org>
    Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef MESSAGEANALYZER_H
#define MESSAGEANALYZER_H

#include <messageviewer/viewer/objecttreeemptysource.h>
#include <messageviewer/viewer/viewer.h>

#include <kmime/kmime_headers.h>
#include <kmime/kmime_message.h>

#include <KDE/KUrl>

#include "mailendanalyzer.h"

namespace MessageViewer {
class NodeHelper;
class ObjectTreeParser;
}

namespace StrigiEndAnalyzer {

class MessageAnalyzer : public QObject, public MessageViewer::EmptySource
{
  Q_OBJECT

  public:
    MessageAnalyzer( const KMime::Message::Ptr &message, MailEndAnalyzer *analyzer );
    ~MessageAnalyzer();

    void analyze();

    /* reimpl from EmptySource */
    virtual QObject* sourceObject() { return this; }

  public Q_SLOTS:
    void update( MessageViewer::Viewer::UpdateMode mode );

  Q_SIGNALS:
    void finished();

  private:
    void processContent( const KMime::Message::Ptr &msg );
    void processHeaders( const KMime::Message::Ptr &msg );
    void processPart( KMime::Content *content );
    void addMailboxValues( MailEndAnalyzer::Field field, const KMime::Types::Mailbox::List &mailboxes );

  private:
    MailEndAnalyzer *m_analyzer;
    KMime::Message::Ptr m_message;
    KMime::Content *m_mainBodyPart;
    MessageViewer::NodeHelper *m_nodeHelper;
    MessageViewer::ObjectTreeParser *m_otp;
};

}

#endif
