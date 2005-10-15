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
namespace KNConfig {
  class IdentityWidget;
}

namespace KNConfig {

class Base {

  public:
    Base() : d_irty(false) {}
    virtual ~Base()        {}

    virtual void save()    {}

    bool dirty()const           { return d_irty; }
    void setDirty(bool b)  { d_irty=b; }

  protected:
    bool d_irty;

};


class KDE_EXPORT Identity : public QObject, public Base {

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
    bool hasName()                    { return (!n_ame.isEmpty()); }
    QString name() const                   { return n_ame; }
    void setName(const QString &s)    { n_ame=s; }
    bool emailIsValid();
    bool hasEmail()                   { return (!e_mail.isEmpty()); }
    QString email()                   { return e_mail; }
    void setEmail(const Q3CString &s)  { e_mail=s; }
    bool hasReplyTo()                 { return (!r_eplyTo.isEmpty()); }
    QString replyTo()                 { return r_eplyTo; }
    void setReplyTo(const QString &s) { r_eplyTo=s; }
    bool hasMailCopiesTo()            { return (!m_ailCopiesTo.isEmpty()); }
    QString mailCopiesTo()            { return m_ailCopiesTo; }
    void setMailCopiesTo(const QString &s) { m_ailCopiesTo=s; }
    bool hasOrga()                    { return (!o_rga.isEmpty()); }
    QString orga() const                   { return o_rga; }
    void setOrga(const QString &s)    { o_rga=s; }

    // OpenPGP signing key
    bool hasSigningKey()                  { return (!s_igningKey.isEmpty()); }
    Q3CString signingKey()                 { return s_igningKey; }
    void setSigningKey(const Q3CString &s) { s_igningKey=s;}

    //signature
    bool hasSignature()       { return ( (u_seSigFile && !s_igPath.isEmpty()) || !s_igText.isEmpty() ); }
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
    Q3CString  s_igningKey;
    bool      u_seSigFile,
              u_seSigGenerator,
              g_lobal;
};


class KDE_EXPORT Appearance : public Base {

#define COL_CNT 16
#define FNT_CNT 5
#define HEX_CNT 4
#define ICON_CNT 14

  friend class AppearanceWidget;

  public:
    enum ColorIndex   { background=0, alternateBackground=1, normalText=2, quoted1=3,
                        quoted2=4, quoted3=5, url=6, unreadThread=7, readThread=8,
                        unreadArticle=9, readArticle=10, signOkKeyOk=11, signOkKeyBad=12,
                        signWarn=13, signErr=14, htmlWarning=15 };

    enum FontIndex    { article=0, articleFixed=1, composer=2, groupList=3, articleList=4 };

    enum IconIndex    { greyBall=0,        redBall=1,      greyBallChkd=2,
                        redBallChkd=3,     newFups=4,      eyes=5,
                        ignore=6,          mail=7,         posting=8,
                        canceledPosting=9, savedRemote=10, group=11,
                        sendErr=12,        null=13 };
    Appearance();
    ~Appearance();

    void save();

    QColor backgroundColor() const;
    QColor alternateBackgroundColor() const;
    QColor textColor() const;
    QColor quoteColor( int depth ) const;
    QColor linkColor() const;
    QColor unreadThreadColor() const;
    QColor readThreadColor() const;
    QColor unreadArticleColor() const;
    QColor readArticleColor() const;
    QColor signOkKeyOkColor() const { return u_seColors ? c_olors[signOkKeyOk] : defaultColor( signOkKeyOk ); }
    QColor signOkKeyBadColor() const { return u_seColors ? c_olors[signOkKeyBad] : defaultColor( signOkKeyBad ); }
    QColor signWarnColor() const { return u_seColors ? c_olors[signWarn] : defaultColor( signWarn ); }
    QColor signErrColor() const { return u_seColors ? c_olors[signErr] : defaultColor( signErr ); }
    QColor htmlWarningColor() const { return u_seColors ? c_olors[htmlWarning] : defaultColor( htmlWarning ); }

    QFont articleFont() const;
    QFont articleFixedFont() const;
    QFont composerFont() const;
    QFont groupListFont() const;
    QFont articleListFont() const;

    const QPixmap& icon(IconIndex i)     { return i_cons[i]; }

  protected:
    const QColor& color( int i ) const    { return c_olors[i]; }
    const QString& colorName( int i ) const { return c_olorNames[i]; }
    int colorCount() const               { return COL_CNT; }
    QColor defaultColor(int i) const;

    const QFont& font(int i) const       { return f_onts[i]; }
    const QString& fontName(int i) const { return f_ontNames[i]; }
    int fontCount() const                { return FNT_CNT; }
    QFont defaultFont(int) const;

    void recreateLVIcons();

    bool u_seColors,
         u_seFonts;
    QColor  c_olors[COL_CNT];
    QString c_olorNames[COL_CNT];
    QFont   f_onts[FNT_CNT];
    QString f_ontNames[FNT_CNT];
    QPixmap i_cons[ICON_CNT];

};


class KDE_EXPORT DisplayedHeaders : public Base
{
  public:
    DisplayedHeaders();
    ~DisplayedHeaders();

    void save();

    KNDisplayedHeader* createNewHeader();
    void remove(KNDisplayedHeader *h);
    void up(KNDisplayedHeader *h);
    void down(KNDisplayedHeader *h);

    Q3ValueList<KNDisplayedHeader*> headers() const { return mHeaderList; }


  protected:
    Q3ValueList<KNDisplayedHeader*> mHeaderList;

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
class KDE_EXPORT PostNewsTechnical : public Base
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

class KDE_EXPORT Cleanup : public Base {

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


/*class Cache : public Base {

  friend class CacheWidget;

  public:
    Cache();
    ~Cache();

    void save();

    // memory-cache
    int memoryMaxArticles()   { return m_emMaxArt; }
    int memoryMaxKBytes()     { return m_emMaxKB; }

    // disk-cache
    int diskMaxArticles()     { return d_iskMaxArt; }
    int diskMaxKBytes()       { return d_iskMaxKB; }


  protected:
    int m_emMaxArt,
        m_emMaxKB,
        d_iskMaxArt,
        d_iskMaxKB;

};*/


} //KNConfig

#endif //KNCONFIG_H
