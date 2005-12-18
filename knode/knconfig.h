/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNCONFIG_H
#define KNCONFIG_H

#include <q3asciidict.h>
#include <qcolor.h>
#include <qdatetime.h>
#include <qfont.h>
#include <QList>
#include <qobject.h>
#include <qpixmap.h>

//Added by qt3to4:
#include <Q3CString>

#include <kconfig.h>

#include <kmime_util.h>

class KScoringRule;
class KProcess;
class KSpellConfig;
namespace Kpgp {
  class Config;
}

class KNNntpAccount;
class KNAccountManager;
class KNArticleFilter;
class KNFilterManager;
class KNDisplayedHeader;
class KNServerInfo;
class KNScoringManager;
namespace KNode {
  class IdentityWidget;
}

namespace KNode {

/** Base class for config settings.
 * @deprecated Use KConfigXT instead.
 */
class ConfigBase {

  public:
    ConfigBase() : d_irty(false) {}
    virtual ~ConfigBase()        {}

    virtual void save()    {}

    bool dirty()const           { return d_irty; }
    void setDirty(bool b)  { d_irty=b; }

  protected:
    bool d_irty;

};


/** A user identity. */
class KDE_EXPORT Identity : public QObject, public ConfigBase {

Q_OBJECT

  friend class IdentityWidget;

  public:
    Identity(bool g=true);
    ~Identity();

    void loadConfig(KConfigBase *c);
    void saveConfig(KConfigBase *c);
    void save();
    bool isEmpty();
    bool isGlobal()const           { return g_lobal; }

    //personal information
    bool hasName() const { return !n_ame.isEmpty(); }
    QString name() const                   { return n_ame; }
    void setName(const QString &s)    { n_ame=s; }
    bool emailIsValid();
    bool hasEmail() const { return !e_mail.isEmpty(); }
    QString email() const { return e_mail; }
    void setEmail( const QString &s ) { e_mail = s; }
    bool hasReplyTo() const { return !r_eplyTo.isEmpty(); }
    QString replyTo() const { return r_eplyTo; }
    void setReplyTo(const QString &s) { r_eplyTo=s; }
    bool hasMailCopiesTo() const { return !m_ailCopiesTo.isEmpty(); }
    QString mailCopiesTo() const { return m_ailCopiesTo; }
    void setMailCopiesTo(const QString &s) { m_ailCopiesTo=s; }
    bool hasOrga() const { return !o_rga.isEmpty(); }
    QString orga() const                   { return o_rga; }
    void setOrga(const QString &s)    { o_rga=s; }

    // OpenPGP signing key
    bool hasSigningKey() const { return !s_igningKey.isEmpty(); }
    QString signingKey() const            { return s_igningKey; }
    void setSigningKey( const QString &s ) { s_igningKey = s;}

    //signature
    bool hasSignature() const { return (u_seSigFile && !s_igPath.isEmpty()) || !s_igText.isEmpty(); }
    bool useSigFile() const        { return u_seSigFile; }
    bool useSigGenerator()const    { return u_seSigGenerator; }
    QString sigPath()const         { return s_igPath; }
    QString sigText()const         { return s_igText; }
    QString getSignature();
    QString getSigGeneratorStdErr() { return s_igStdErr; }


  protected slots:
    void slotReceiveStdout(KProcess *proc, char *buffer, int buflen);
    void slotReceiveStderr(KProcess *proc, char *buffer, int buflen);

  protected:
    QString   n_ame,
              e_mail,
              o_rga,
              r_eplyTo,
              m_ailCopiesTo,
              s_igText,
              s_igContents,
              s_igStdErr,
              s_igPath;
    QString  s_igningKey;
    bool      u_seSigFile,
              u_seSigGenerator,
              g_lobal;
};


/** Ex-Appearance settings.
 * @deprecated Move the remaining parts either to their only users or to the
 *  KConfigXT generated Settings class.
 */
class KDE_EXPORT Appearance : public ConfigBase {

#define ICON_CNT 14

  friend class AppearanceWidget;

  public:
    enum IconIndex    { greyBall=0,        redBall=1,      greyBallChkd=2,
                        redBallChkd=3,     newFups=4,      eyes=5,
                        ignore=6,          mail=7,         posting=8,
                        canceledPosting=9, savedRemote=10, group=11,
                        null=12 };
    Appearance();

    const QPixmap& icon(IconIndex i)     { return i_cons[i]; }

  protected:
    void recreateLVIcons();

    QPixmap i_cons[ICON_CNT];

};


/** Headers displayed in the article viewer. */
class KDE_EXPORT DisplayedHeaders : public ConfigBase
{
  public:
    DisplayedHeaders();
    ~DisplayedHeaders();

    void save();

    KNDisplayedHeader* createNewHeader();
    void remove(KNDisplayedHeader *h);
    void up(KNDisplayedHeader *h);
    void down(KNDisplayedHeader *h);

    /** Returns the list of headers displayed in the article viewer. */
    QList<KNDisplayedHeader*> headers() const { return mHeaderList; }


  protected:
    QList<KNDisplayedHeader*> mHeaderList;

};


/** Represents an additional header added by the composer. */
class KDE_EXPORT XHeader
{
  public:
    /** Create a new XHeader object from the given string representation.
     * @param s String representation of a MIME header (i.e. "Name: Value").
     */
    XHeader( const QString &s );

    /** Returns the header name. */
    QString name() const { return mName; }
    /** Returns the header value. */
    QString value() const { return mValue; }
    /** Returns a string representation of the header, ie. "Name: Value". */
    QString header() const { return mName + ": " + mValue; }

  private:
    QString mName;
    QString mValue;
};

/** A list of additional headers. */
typedef QList<XHeader> XHeaders;


/** Technical posting settings. */
class KDE_EXPORT PostNewsTechnical : public ConfigBase
{
  friend class PostNewsTechnicalWidget;

  public:
    PostNewsTechnical();
    ~PostNewsTechnical();

    void save();

    Q3CString charset() const         { return c_harset; }
    QStringList composerCharsets() { return c_omposerCharsets; }
    int indexForCharset(const Q3CString &str);
    Q3CString findComposerCharset(Q3CString cs);

    /** Returns a list of additional headers. */
    XHeaders xHeaders() const { return mXheaders; }

  protected:
    Q3CString  c_harset;
    QStringList c_omposerCharsets;
    XHeaders mXheaders;

    Q3AsciiDict<Q3CString> findComposerCSCache;
};


//BEGIN: Cleanup configuration -----------------------------------------------

/** Expirery/cleaup settings (globally or per account/group/folder). */
class KDE_EXPORT Cleanup : public ConfigBase {

  friend class CleanupWidget;
  friend class GroupCleanupWidget;

  public:
    Cleanup( bool global = true );
    ~Cleanup() {}

    void loadConfig( KConfigBase *conf );
    void saveConfig( KConfigBase *conf );
    void save();

    //expire
    int maxAgeForRead() const        { return r_eadMaxAge; }
    int maxAgeForUnread() const      { return u_nreadMaxAge; }
    bool removeUnavailable() const   { return r_emoveUnavailable; }
    bool preserveThreads() const     { return p_reserveThr; }
    bool isGlobal() const            { return mGlobal; }
    bool useDefault() const          { return mDefault; }
    bool expireToday();
    void setLastExpireDate();

    void setUseDefault( bool def )   { mDefault = def; }

    //compact
    bool compactToday();
    void setLastCompactDate();


  protected:
    bool  d_oExpire,
          r_emoveUnavailable,
          p_reserveThr,
          d_oCompact;
    int   e_xpireInterval,
          r_eadMaxAge,
          u_nreadMaxAge,
          c_ompactInterval;

  private:
    /** global vs. per account or per group configuration */
    bool mGlobal;
    /** use default cleanup configuration */
    bool mDefault;
    /** last expiration and last comapction date */
    QDate mLastExpDate, mLastCompDate;

};

//END: Cleanup configuration -------------------------------------------------

} //KNode

#endif //KNCONFIG_H
