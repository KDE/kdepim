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

#include <qasciidict.h>
#include <qcolor.h>
#include <qdatetime.h>
#include <qfont.h>
#include <qobject.h>
#include <qpixmap.h>

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
    void setEmail(const QCString &s)  { e_mail=s; }
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
    QCString signingKey()                 { return s_igningKey; }
    void setSigningKey(const QCString &s) { s_igningKey=s;}

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
    QCString  s_igningKey;
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


class KDE_EXPORT ReadNewsGeneral : public Base {

  friend class ReadNewsGeneralWidget;

  public:
    ReadNewsGeneral();
    ~ReadNewsGeneral();

    void save();

    bool autoCheckGroups()const           { return a_utoCheck; }
    int maxToFetch()const                 { return m_axFetch; }
    bool autoMark()const                  { return a_utoMark; }
    int autoMarkSeconds()const            { return m_arkSecs; }
    bool markCrossposts()const            { return m_arkCrossposts; }

    bool smartScrolling()const            { return s_martScrolling; }
    bool totalExpandThreads()const        { return t_otalExpand; }
    bool defaultToExpandedThreads()const  { return d_efaultExpand; }
    bool showLines()const                 { return s_howLines; }
    bool showScore()const                 { return s_howScore; }
    bool showUnread()const                { return s_howUnread; }

    int collCacheSize()const              { return c_ollCacheSize; }
    int artCacheSize()const               { return a_rtCacheSize; }

    bool showThreads()const               { return s_howThreads; }
    void setShowThreads(bool b)      { d_irty=true; s_howThreads=b;}

    KMime::DateFormatter::FormatType dateFormat() const { return mDateFormat; }
    QString dateCustomFormat() const { return mDateCustomFormat; }

    void setShowLines( bool show ) { d_irty = true; s_howLines = show; }
    void setShowScore( bool show ) { d_irty = true; s_howScore = show; }

  protected:
    bool  a_utoCheck,
          a_utoMark,
          m_arkCrossposts,
          s_martScrolling,
          t_otalExpand,
          d_efaultExpand,
          s_howLines,
          s_howScore,
          s_howUnread,
          s_howThreads;

    int   m_axFetch,
          m_arkSecs,
          c_ollCacheSize,
          a_rtCacheSize;

    KMime::DateFormatter::FormatType mDateFormat;
    QString mDateCustomFormat;

};


class KDE_EXPORT ReadNewsNavigation : public Base {

  friend class ReadNewsNavigationWidget;

  public:
    ReadNewsNavigation();
    ~ReadNewsNavigation();

    void save();

    bool markAllReadGoNext()const          { return m_arkAllReadGoNext; }
    bool markThreadReadGoNext() const      { return m_arkThreadReadGoNext; }
    bool markThreadReadCloseThread()const  { return m_arkThreadReadCloseThread; }
    bool ignoreThreadGoNext()const         { return i_gnoreThreadGoNext; }
    bool ignoreThreadCloseThread()const   { return i_gnoreThreadCloseThread; }
    bool leaveGroupMarkAsRead() const    { return mLeaveGroupMarkAsRead; }

  protected:
    bool  m_arkAllReadGoNext,
          m_arkThreadReadGoNext,
          m_arkThreadReadCloseThread,
          i_gnoreThreadGoNext,
          i_gnoreThreadCloseThread,
          mLeaveGroupMarkAsRead;

};


class KDE_EXPORT ReadNewsViewer : public Base {

  friend class ReadNewsViewerWidget;

  public:
    ReadNewsViewer();
    ~ReadNewsViewer();

    void save();

    bool rewrapBody()const                { return r_ewrapBody; }
    bool removeTrailingNewlines()const    { return r_emoveTrailingNewlines; }
    bool showSignature()const             { return s_howSig; }
    bool interpretFormatTags()const       { return i_nterpretFormatTags; }
    void setInterpretFormatTags( bool f ) { d_irty = true; i_nterpretFormatTags = f; }
    QString quoteCharacters()const        { return q_uoteCharacters; }

    bool openAttachmentsOnClick()const    { return o_penAtt; }
    bool showAlternativeContents()const   { return s_howAlts; }

    bool useFixedFont() const             { return u_seFixedFont; }
    void setUseFixedFont(bool b)     { d_irty = true; u_seFixedFont=b; }

    bool showRefBar() const                { return mShowRefBar; }
    void setShowRefBar( bool b )           { d_irty = true; mShowRefBar = b; }

    bool alwaysShowHTML() const            { return mAlwaysShowHTML; }
    void setAlwaysShowHTML( bool b )       { d_irty = true; mAlwaysShowHTML = b; }

  protected:
    bool  r_ewrapBody,
          r_emoveTrailingNewlines,
          s_howSig,
          i_nterpretFormatTags,
          o_penAtt,
          s_howAlts,
          u_seFixedFont,
          mShowRefBar,
          mAlwaysShowHTML;
    QString q_uoteCharacters;
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

    QValueList<KNDisplayedHeader*> headers() const { return mHeaderList; }


  protected:
    QValueList<KNDisplayedHeader*> mHeaderList;

};


class KDE_EXPORT Scoring : public Base {

  friend class ScoringWidget;

  public:
    Scoring();
    ~Scoring();

    void save();

    int ignoredThreshold()   { return i_gnoredThreshold; }
    int watchedThreshold()   { return w_atchedThreshold; }

  protected:
    int i_gnoredThreshold,
        w_atchedThreshold;

};


class KDE_EXPORT XHeader {

  public:
    XHeader()                  {}
    XHeader(const QString &s);
    XHeader(const XHeader &s)  { n_ame=s.n_ame; v_alue=s.v_alue; }
    ~XHeader()                 {}

    XHeader& operator=(const XHeader &s) { n_ame=s.n_ame; v_alue=s.v_alue; return (*this); }

    QCString name()   { return n_ame; }
    QString value()   { return v_alue; }
    QString header()  { return (QString::fromLatin1(("X-"+n_ame+": "))+v_alue); }

  protected:
    QCString n_ame;
    QString v_alue;

};
typedef QValueList<XHeader> XHeaders;


class KDE_EXPORT PostNewsTechnical : public Base {

  friend class PostNewsTechnicalWidget;
  friend class SmtpAccountWidget;

  public:
    PostNewsTechnical();
    ~PostNewsTechnical();

    void save();

    QCString charset() const         { return c_harset; }
    QStringList composerCharsets() { return c_omposerCharsets; }
    int indexForCharset(const QCString &str);
    QCString findComposerCharset(QCString cs);

    bool allow8BitBody() const       { return a_llow8BitBody; }
    bool useOwnCharset() const       { return u_seOwnCharset; }
    bool generateMessageID()const    { return g_enerateMID; }
    QCString hostname()const         { return h_ostname; }
    XHeaders& xHeaders()        { return x_headers; }
    bool noUserAgent()const          { return d_ontIncludeUA; }
    bool useExternalMailer()const    { return u_seExternalMailer; }

  protected:
    QCString  c_harset,
              h_ostname;
    QStringList c_omposerCharsets;

    bool      a_llow8BitBody,
              u_seOwnCharset,
              g_enerateMID,
              d_ontIncludeUA,
              u_seExternalMailer;

    XHeaders x_headers;

    QAsciiDict<QCString> findComposerCSCache;
};


class PostNewsComposer : public Base {

  friend class PostNewsComposerWidget;

  public:
    PostNewsComposer();
    ~PostNewsComposer();

    void save();

    bool wordWrap()const             { return w_ordWrap; }
    int maxLineLength()const         { return m_axLen; }
    bool appendOwnSignature()const   { return a_ppSig; }

    QString intro()const             { return i_ntro; }
    bool rewrap()const               { return r_ewrap; }
    bool includeSignature()const     { return i_ncSig; }
    bool cursorOnTop()const          { return c_ursorOnTop; }

    QString externalEditor()const    { return e_xternalEditor; }
    bool useExternalEditor()const    { return u_seExtEditor; }


  protected:
    int     m_axLen;
    bool    w_ordWrap,
            a_ppSig,
            r_ewrap,
            i_ncSig,
            c_ursorOnTop,
            u_seExtEditor;
    QString i_ntro,
            e_xternalEditor;

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
    QDateTime mLastExpDate, mLastCompDate;

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
