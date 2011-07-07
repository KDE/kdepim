/*   -*- mode: C++; c-file-style: "gnu" -*-
 *   kmail: KDE mail client
 *   Copyright (C) 2006 Dmitry Morozhnikov <dmiceman@mail.ru>
 *   Copyright (C) 2011 Sudhendu Kumar <sudhendu.kumar.roy@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef __KMAIL_TEMPLATEPARSER_H__
#define __KMAIL_TEMPLATEPARSER_H__

#include "templateparser_export.h"

#include <qobject.h>
#include <akonadi/collection.h>

#include <kmime/kmime_message.h>
#include <boost/shared_ptr.hpp>

namespace MessageViewer {
  class ObjectTreeParser;
}

namespace KPIMIdentities {
  class IdentityManager;
}

class QString;
class QObject;

namespace TemplateParser {

/**
 * The TemplateParser transforms a message with a given template.
 *
 * A template contains text and commands, such as %QUOTE or %ODATE, which will be
 * replaced with the real values in process().
 *
 * The message given in the constructor is the message that is being transformed.
 * The message text will be replaced by the processed text of the template, but other
 * properties, such as the attachments or the subject, are preserved.
 *
 * There are two different kind of commands: Those that work on the message that is
 * to be transformed and those that work on an 'original message'.
 * Those that work on the message that is to be transformed have no special prefix, e.g.
 * '%DATE'. Those that work on the original message have an 'O' prefix, for example
 * '%ODATE'.
 * This means that the %DATE command will take the date of the message passed in the
 * constructor, the message which is to be transformed, whereas the %ODATE command will
 * take the date of the message that is being passed in process(), the original message.
 *
 * TODO: What is the usecase of the commands that work on the message to be transformed?
 *       In general you only use the commands that work on the original message...
 */
class TEMPLATEPARSER_EXPORT TemplateParser : public QObject
{
  Q_OBJECT

  public:
    enum Mode {
      NewMessage,
      Reply,
      ReplyAll,
      Forward
    };

  public:
    TemplateParser( const KMime::Message::Ptr &amsg, const Mode amode );
    ~TemplateParser();

    /**
     * Sets the selection. If this is set, only the selection will be added to commands such
     * as %QUOTE. Otherwise, the whole message is quoted.
     * If this is not called at all, the whole message is quoted as well.
     * Call this before calling process().
     */
    void setSelection( const QString &selection );

    /**
     * Sets whether the template parser is allowed to decrypt the original message when needing
     * its message text, for example for the %QUOTE command.
     * If true, it will tell the ObjectTreeParser it uses internally to decrypt the message,
     * and that will possibly show a password request dialog to the user.
     *
     * The default is false.
     */
    void setAllowDecryption( const bool allowDecryption );

    /**
     * Tell template parser whether or not to wrap words, and
     *  at what column to wrap at.
     *
     * Default is true, wrapping at 80chars.
     */
    void setWordWrap( bool wrap, int wrapColWidth = 80 );

    
    /**
     * Set the identity manager to be used when creating the template.
     */
    void setIdentityManager( KPIMIdentities::IdentityManager* ident );

    /**
     * Sets the list of charsets to try to use to encode the resulting
     *  text. They are tried in order until one matches, or utf-8 as a fallback.
     */
    void setCharsets( const QStringList& charsets );

    virtual void process( const KMime::Message::Ptr &aorig_msg,
                          const Akonadi::Collection& afolder = Akonadi::Collection() );
    virtual void process( const QString &tmplName, const KMime::Message::Ptr &aorig_msg,
                         const Akonadi::Collection& afolder = Akonadi::Collection() );
    virtual void processWithIdentity( uint uoid, const KMime::Message::Ptr &aorig_msg,
                                      const Akonadi::Collection& afolder = Akonadi::Collection() );

    virtual void processWithTemplate( const QString &tmpl );

    /// This finds the template to use. Either the one from the folder, identity or
    /// finally the global template.
    /// This also reads the To and CC address of the template
    /// @return the contents of the template
    virtual QString findTemplate();

    /// Finds the template with the given name.
    /// This also reads the To and CC address of the template
    /// @return the contents of the template
    virtual QString findCustomTemplate( const QString &tmpl );

    virtual QString pipe( const QString &cmd, const QString &buf );

    virtual QString getFName( const QString &str );
    virtual QString getLName( const QString &str );

  protected:
    Mode mMode;
    Akonadi::Collection mFolder;
    uint mIdentity;
    KMime::Message::Ptr mMsg;
    KMime::Message::Ptr mOrigMsg;
    QString mSelection;
    bool mAllowDecryption;
    bool mDebug;
    QString mQuoteString;
    QString mTo, mCC;
    KMime::Content *mOrigRoot;
    KPIMIdentities::IdentityManager* m_identityManager;
    bool mWrap;
    int mColWrap;
    QStringList m_charsets;

    /**
     * Called by processWithTemplate(). This adds the completely processed body to
     * the message.
     *
     * This function creates plain text message or multipart/alternative message,
     * depending on whether the processed body has @p htmlBody or not.
     *
     * In append mode, this will simply append the text to the body.
     *
     * Otherwise, the content of the old message is deleted and replaced with @p plainBody
     * and @p htmlBody.
     * Attachments of the original message are also added back to the new message.
     */
    void addProcessedBodyToMessage( const QString &plainBody, const QString &htmlBody, KMime::Content *root );

    /**
     * Determines whether the signature should be stripped when getting the text of the original
     * message, e.g. for commands such as %QUOTE
     */
    bool shouldStripSignature() const;

    int parseQuotes( const QString &prefix, const QString &str,
                     QString &quote ) const;

  private:
    /**
     * Return the text signature used the by current identity.
     */
    QString getSignature() const;

    /**
      * Returns message body indented by the
      * given indentation string. This is suitable for including the message
      * in another message of for replies, forwards.
      *
      * No attachments are handled if includeAttach is false.
      * The signature is stripped if aStripSignature is true and
      * smart quoting is turned on. Signed or encrypted texts
      * get converted to plain text when allowDecryption is true.
    */
    QString asQuotedString( const KMime::Message::Ptr &msg,
                            const QString &indentStr,
                            const QString & election=QString(),
                            bool aStripSignature=true,
                            bool allowDecryption=true);

    /**
     * This function return the plain text part from the OTP.
     * For HTML only mails. It returns the converted plain text
     * from the OTP.
     * @param allowSelectionOnly takes care that if a reply/forward
     * is made to a selected part of message, then the selection is
     * returned as it is without going through th OTP
     * @param aStripSignature strips the signature out of the message
     *
     */
    QString plainMessageText( const KMime::Message::Ptr &msg,
                              MessageViewer::ObjectTreeParser *otp,
                              bool aStripSignature, bool allowDecryption,
                              bool allowSelectionOnly = false );

    /** @return the UOID of the identity for this message.
      Searches the "x-kmail-identity" header and if that fails,
      searches with KPIMIdentities::IdentityManager::identityForAddress()
    **/
    uint identityUoid(const KMime::Message::Ptr &msg );

    /**
     * Returns KMime content of the plain text part of the message after setting its mime type,
     * charset and CTE.
     * This function is called by:-
     * 1) TemplateParser::addProcessedBodyToMessage(), which uses this content to simply create
     *    the text/plain message
     *
     * 2) TemplateParser::createMultipartAlternativeContent() which adds this content to create
     *    multipart/alternative message.
     */
    KMime::Content* createPlainPartContent( const QString &plainBody ) const;

    /**
     * Returns KMime content of the multipart/alternative part of the message after setting the
     * mime type, charset and CTE of its respective text/plain part and text/html part.
     */
    KMime::Content* createMultipartAlternativeContent( const QString &plainBody, const QString &htmlBody ) const;

};

} // namespace TemplateParser

#endif // __KMAIL_TEMPLATEPARSER_H__
