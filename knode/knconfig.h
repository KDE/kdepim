/*
    knconfig.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2004 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNCONFIG_H
#define KNCONFIG_H

#include <qasciidict.h>
#include <qdatetime.h>

#include <kdialogbase.h>
#include <kcmodule.h>
#include <kmime_util.h>

#include "knwidgets.h"

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QRadioButton;
class QTextEdit;

class KScoringRule;
class KScoringEditorWidget;
class KConfigBase;
class KProcess;
class KLineEdit;
class KComboBox;
class KIntSpinBox;
class KSpellConfig;
class KURLCompletion;
namespace Kpgp {
  class Config;
  class SecretKeyRequester;
}
namespace KNConfig {
  class GroupCleanupWidget;
}

class KNNntpAccount;
class KNAccountManager;
class KNArticleFilter;
class KNFilterManager;
class KNDisplayedHeader;
class KNServerInfo;
class KNScoringManager;

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


class BaseWidget : public KCModule {

  Q_OBJECT

  public:
    BaseWidget(QWidget *p=0, const char *n=0) : KCModule(p, n), d_irty(false) {}
    ~BaseWidget() {}

    void show()             { d_irty=true; QWidget::show(); }

    bool dirty()const            { return d_irty; }
    void setDirty(bool b)   { d_irty=b; }

  protected:
    bool d_irty;

  protected slots:
    void slotEmitChanged() { emit changed( true ); }
};


class Identity : public QObject, public Base {

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


class IdentityWidget : public BaseWidget {

  Q_OBJECT

  public:
    IdentityWidget(Identity *d, QWidget *p=0, const char *n=0);
    ~IdentityWidget();

    void load();
    void save();

  protected:
    QLabel          *f_ileName;
    KLineEdit       *n_ame,
                    *o_rga,
                    *e_mail,
                    *r_eplyTo,
                    *m_ailCopiesTo,
                    *s_ig;
    QRadioButton    *s_igFile,
                    *s_igEdit;
    QCheckBox       *s_igGenerator;
    QPushButton     *c_hooseBtn,
                    *e_ditBtn;
    QTextEdit       *s_igEditor;
    QButtonGroup    *b_uttonGroup;
    Kpgp::SecretKeyRequester
                    *s_igningKey;
    KURLCompletion  *c_ompletion;

    Identity        *d_ata;

  protected slots:
    void slotSignatureType(int type);
    void slotSignatureChoose();
    void slotSignatureEdit();
    void textFileNameChanged(const QString &);

};


class NntpAccountListWidget : public BaseWidget {

  Q_OBJECT

  public:
    NntpAccountListWidget(QWidget *p=0, const char *n=0);
    ~NntpAccountListWidget();

    void load();

  protected:
    class LBoxItem : public KNListBoxItem {
      public:
        LBoxItem(KNNntpAccount *a, const QString &t, QPixmap *p=0)
          : KNListBoxItem(t, p) , account(a)  {}
        ~LBoxItem() {}
        KNNntpAccount *account;
    };

    KNDialogListBox *l_box;
    QPushButton *a_ddBtn,
                *d_elBtn,
                *e_ditBtn,
                *s_ubBtn;
    QPixmap     p_ixmap;
    QLabel      *s_erverInfo,
                *p_ortInfo;

    KNAccountManager *a_ccManager;


  public slots:
    void slotAddItem(KNNntpAccount *a);
    void slotRemoveItem(KNNntpAccount *a);
    void slotUpdateItem(KNNntpAccount *a);

  protected slots:
    void slotSelectionChanged();
    void slotItemSelected(int id);
    void slotAddBtnClicked();
    void slotDelBtnClicked();
    void slotEditBtnClicked();
    void slotSubBtnClicked();

};


class NntpAccountConfDialog : public KDialogBase  {

  Q_OBJECT

  public:
    NntpAccountConfDialog(KNNntpAccount* acc, QWidget *p=0, const char *n=0);
    ~NntpAccountConfDialog();

  protected:
    KLineEdit   *n_ame,
                *s_erver,
                *u_ser,
                *p_ass,
                *p_ort;
    QLabel      *u_serLabel,
                *p_assLabel,
                *c_heckIntervalLabel;
    KIntSpinBox *h_old,
                *t_imeout,
                *c_heckInterval;
    QCheckBox   *f_etchDes,
                *a_uth,
                *u_seDiskCache,
                *i_nterval;
    KNConfig::IdentityWidget* i_dWidget;

    KNNntpAccount *a_ccount;

  protected slots:
    void slotOk();
    void slotAuthChecked(bool b);
    void slotIntervalChecked(bool b);

  private:
    GroupCleanupWidget *mCleanupWidget;
};


class SmtpAccountWidget : public BaseWidget {

Q_OBJECT

  public:
    SmtpAccountWidget(QWidget *p=0, const char *n=0);
    ~SmtpAccountWidget();

    void load();
    void save();

  protected slots:
    void useExternalMailerToggled(bool b);

  protected:
    KNServerInfo  *s_erverInfo;
    QCheckBox     *u_seExternalMailer;
    KLineEdit     *s_erver,
                  *p_ort;
    KIntSpinBox   *h_old,
                  *t_imeout;
    QLabel        *s_erverLabel,
                  *p_ortLabel,
                  *h_oldLabel,
                  *t_imeoutLabel;
};


class Appearance : public Base {

#define COL_CNT 12
#define FNT_CNT 5
#define HEX_CNT 4
#define ICON_CNT 14

  friend class AppearanceWidget;

  public:
    enum ColorIndex   { background=0, alternateBackground=1, header=2, normalText=3, quoted1=4,
                        quoted2=5, quoted3=6, url=7, unreadThread=8, readThread=9,
                        unreadArticle=10, readArticle=11 };

    enum HexIndex     { quoted1Hex=0, quoted2Hex=1, quoted3Hex=2 };

    enum FontIndex    { article=0, articleFixed=1, composer=2, groupList=3, articleList=4 };

    enum IconIndex    { greyBall=0,        redBall=1,      greyBallChkd=2,
                        redBallChkd=3,     newFups=4,      eyes=5,
                        ignore=6,          mail=7,         posting=8,
                        canceledPosting=9, savedRemote=10, group=11,
                        sendErr=12,        null=13 };
    Appearance();
    ~Appearance();

    void save();

    QColor backgroundColor();
    QColor alternateBackgroundColor();
    QColor textColor();
    QColor quoteColor1();
    QColor quoteColor2();
    QColor quoteColor3();
    QColor linkColor();
    QColor headerDecoColor();
    QColor unreadThreadColor();
    QColor readThreadColor();
    QColor unreadArticleColor();
    QColor readArticleColor();

    QString headerDecoHexcode()       { return headerDecoColor().name(); }
    QString quotedTextHexcode(int i);

    QFont articleFont();
    QFont articleFixedFont();
    QFont composerFont();
    QFont groupListFont();
    QFont articleListFont();

    const QPixmap& icon(IconIndex i)     { return i_cons[i]; }

  protected:
    const QColor& color(int i)           { return c_olors[i]; }
    const QString& colorName(int i)      { return c_olorNames[i]; }
    int colorCount()                     { return COL_CNT; }
    QColor defaultColor(int i);

    const QFont& font(int i)             { return f_onts[i]; }
    const QString& fontName(int i)       { return f_ontNames[i]; }
    int fontCount()                      { return FNT_CNT; }
    QFont defaultFont(int);

    void recreateLVIcons();

    bool u_seColors,
         u_seFonts;
    QColor  c_olors[COL_CNT];
    QString c_olorNames[COL_CNT];
    QFont   f_onts[FNT_CNT];
    QString f_ontNames[FNT_CNT];
    QPixmap i_cons[ICON_CNT];

};


class AppearanceWidget : public BaseWidget {

  Q_OBJECT

  public:
    AppearanceWidget(QWidget *p=0, const char *n=0);
    ~AppearanceWidget();

    void load();
    void save();
    void defaults();

    //===================================================================================
    // code taken from KMail, Copyright (C) 2000 Espen Sand, espen@kde.org

    class ColorListItem : public QListBoxText {

      public:
        ColorListItem( const QString &text, const QColor &color=Qt::black );
        ~ColorListItem();
        const QColor& color()                     { return mColor; }
        void  setColor( const QColor &color )     { mColor = color; }

      protected:
        virtual void paint( QPainter * );
        virtual int height( const QListBox * ) const;
        virtual int width( const QListBox * ) const;

      private:
        QColor mColor;
    };

    //===================================================================================

    class FontListItem : public QListBoxText {

      public:
        FontListItem( const QString &name, const QFont & );
        ~FontListItem();
        const QFont& font()                     { return f_ont; }
        void setFont( const QFont &);

      protected:
        virtual void paint( QPainter * );
        virtual int width( const QListBox * ) const;

      private:
        QFont f_ont;
        QString fontInfo;
    };

    //===================================================================================

    KNDialogListBox *c_List,
              *f_List;
    QCheckBox *c_olorCB,
              *f_ontCB;
    QPushButton *c_olChngBtn,
                *f_ntChngBtn;

    Appearance *d_ata;

  protected slots:
    //colors
    void slotColCheckBoxToggled(bool b);
    void slotColItemSelected(QListBoxItem *);   // show color dialog for the entry
    void slotColChangeBtnClicked();
    void slotColSelectionChanged();

    //fonts
    void slotFontCheckBoxToggled(bool b);
    void slotFontItemSelected(QListBoxItem *);  // show font dialog for the entry
    void slotFontChangeBtnClicked();
    void slotFontSelectionChanged();

};


class ReadNewsGeneral : public Base {

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

    bool autoCheckPgpSigs()const          { return a_utoCheckPgpSigs; }
    void setAutoCheckPgpSigs(bool b) { d_irty=true; a_utoCheckPgpSigs=b;}

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
          s_howThreads,
          a_utoCheckPgpSigs;

    int   m_axFetch,
          m_arkSecs,
          c_ollCacheSize,
          a_rtCacheSize;

    KMime::DateFormatter::FormatType mDateFormat;
    QString mDateCustomFormat;

};


class ReadNewsGeneralWidget : public BaseWidget {

  public:
    ReadNewsGeneralWidget(ReadNewsGeneral *d, QWidget *p=0, const char *n=0);
    ~ReadNewsGeneralWidget();

    void load();
    void save();

  protected:
    QCheckBox   *a_utoCB,
                *m_arkCB,
                *m_arkCrossCB,
                *s_martScrollingCB,
                *e_xpThrCB,
                *d_efaultExpandCB,
                *l_inesCB,
                *u_nreadCB,
                *s_coreCB;
    KIntSpinBox *m_arkSecs,
                *m_axFetch,
                *c_ollCacheSize,
                *a_rtCacheSize;

    ReadNewsGeneral *d_ata;

};


class ReadNewsNavigation : public Base {

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

  protected:
    bool  m_arkAllReadGoNext,
          m_arkThreadReadGoNext,
          m_arkThreadReadCloseThread,
          i_gnoreThreadGoNext,
          i_gnoreThreadCloseThread;

};


class ReadNewsNavigationWidget : public BaseWidget {

  public:
    ReadNewsNavigationWidget(ReadNewsNavigation *d, QWidget *p=0, const char *n=0);
    ~ReadNewsNavigationWidget();

    void load();
    void save();

  protected:
    QCheckBox   *m_arkAllReadGoNextCB,
                *m_arkThreadReadGoNextCB,
                *m_arkThreadReadCloseThreadCB,
                *i_gnoreThreadGoNextCB,
                *i_gnoreThreadCloseThreadCB;

    ReadNewsNavigation *d_ata;

};


class ReadNewsViewer : public Base {

  friend class ReadNewsViewerWidget;

  public:
    enum browserType { BTdefault=0, BTkonq=1, BTnetscape=2, BTmozilla=3, BTopera=4, BTother=5 };

    ReadNewsViewer();
    ~ReadNewsViewer();

    void save();

    bool showHeaderDecoration()const      { return s_howHeaderDeco; }
    bool rewrapBody()const                { return r_ewrapBody; }
    bool removeTrailingNewlines()const    { return r_emoveTrailingNewlines; }
    bool showSignature()const             { return s_howSig; }
    bool interpretFormatTags()const       { return i_nterpretFormatTags; }
    QString quoteCharacters()const        { return q_uoteCharacters; }

    bool showAttachmentsInline()const     { return i_nlineAtt; }
    bool openAttachmentsOnClick()const    { return o_penAtt; }
    bool showAlternativeContents()const   { return s_howAlts; }

    browserType browser()const            { return b_rowser; }
    QString browserCommand()const         { return b_rowserCommand; }

    bool showFullHdrs()const              { return f_ullHdrs; }
    void setShowFullHdrs(bool b)     { d_irty = true; f_ullHdrs=b; }
    bool useFixedFont() const             { return u_seFixedFont; }
    void setUseFixedFont(bool b)     { d_irty = true; u_seFixedFont=b; }

  protected:
    bool  s_howHeaderDeco,
          r_ewrapBody,
          r_emoveTrailingNewlines,
          s_howSig,
          i_nterpretFormatTags,
          i_nlineAtt,
          o_penAtt,
          s_howAlts,
          f_ullHdrs,
          u_seFixedFont;
    QString q_uoteCharacters;

    browserType b_rowser;
    QString b_rowserCommand;

};


class ReadNewsViewerWidget : public BaseWidget {

  Q_OBJECT

  public:
    ReadNewsViewerWidget(ReadNewsViewer *d, QWidget *p=0, const char *n=0);
    ~ReadNewsViewerWidget();

    void load();
    void save();

  protected:
    QCheckBox   *d_ecoCB,
                *r_ewrapCB,
                *r_emoveTrailingCB,
                *s_igCB,
                *i_nlineCB,
                *o_penAttCB,
                *a_ltAttCB,
                *f_ormatCB;
    QComboBox   *b_rowser;
    KLineEdit   *b_rowserCommand,
                *q_uoteCharacters;
    QPushButton *c_hooseBrowser;

    ReadNewsViewer *d_ata;

  protected slots:
    void slotBrowserTypeChanged(int);
    void slotChooseBrowser();

};



class DisplayedHeaders : public Base {

  friend class DisplayedHeadersWidget;

  public:
    typedef QPtrListIterator<KNDisplayedHeader> Iterator;

    DisplayedHeaders();
    ~DisplayedHeaders();

    void save();

    KNDisplayedHeader* createNewHeader();
    void remove(KNDisplayedHeader *h);
    void up(KNDisplayedHeader *h);
    void down(KNDisplayedHeader *h);

    Iterator iterator()   { return Iterator(h_drList); }


  protected:
    QPtrList<KNDisplayedHeader> h_drList;

};


class DisplayedHeadersWidget : public BaseWidget {

  Q_OBJECT

  public:
    DisplayedHeadersWidget(DisplayedHeaders *d, QWidget *p=0, const char *n=0);
    ~DisplayedHeadersWidget();

    void load();
    void save();

  protected:

     class HdrItem : public QListBoxText {

      public:
        HdrItem( const QString &t, KNDisplayedHeader *h ) : QListBoxText(t), hdr(h) {}
        ~HdrItem() {}

        KNDisplayedHeader *hdr;
    };

    HdrItem* generateItem(KNDisplayedHeader *);

    KNDialogListBox *l_box;
    QPushButton *a_ddBtn,
                *d_elBtn,
                *e_ditBtn,
                *u_pBtn,
                *d_ownBtn;
    bool s_ave;

    DisplayedHeaders *d_ata;

  protected slots:
    void slotItemSelected(int);
    void slotSelectionChanged();
    void slotAddBtnClicked();
    void slotDelBtnClicked();
    void slotEditBtnClicked();
    void slotUpBtnClicked();
    void slotDownBtnClicked();

};


class DisplayedHeaderConfDialog : public KDialogBase {

  Q_OBJECT

  public:
    DisplayedHeaderConfDialog(KNDisplayedHeader *h, QWidget *p=0, char *n=0);
    ~DisplayedHeaderConfDialog();


  protected:
    KNDisplayedHeader *h_dr;
    KComboBox *h_drC;
    KLineEdit *n_ameE;
    QCheckBox *n_ameCB[4],
              *v_alueCB[4];


  protected slots:
    void slotOk();
    void slotActivated(int);
    void slotNameChanged(const QString&);
};


class Scoring : public Base {

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


class ScoringWidget : public BaseWidget {

  Q_OBJECT

  public:
    ScoringWidget(Scoring *d, QWidget *p=0, const char *n=0);
    ~ScoringWidget();

    void load();
    void save();

  private:
    KScoringEditorWidget *ksc;
    KIntSpinBox *i_gnored,
                *w_atched;

    Scoring *d_ata;
};


class FilterListWidget : public BaseWidget {

  Q_OBJECT

  public:
    FilterListWidget(QWidget *p=0, const char *n=0);
    ~FilterListWidget();

    void load();
    void save();

    void addItem(KNArticleFilter *f);
    void removeItem(KNArticleFilter *f);
    void updateItem(KNArticleFilter *f);
    void addMenuItem(KNArticleFilter *f);
    void removeMenuItem(KNArticleFilter *f);
    QValueList<int> menuOrder();


  protected:
    class LBoxItem : public KNListBoxItem {
      public:
        LBoxItem(KNArticleFilter *f, const QString &t, QPixmap *p=0)
          : KNListBoxItem(t, p) , filter(f) {}
        ~LBoxItem() {}

        KNArticleFilter *filter;
    };

    int findItem(QListBox *l, KNArticleFilter *f);

    KNDialogListBox *f_lb,
                    *m_lb;

    QPushButton   *a_ddBtn,
                  *d_elBtn,
                  *e_ditBtn,
                  *c_opyBtn,
                  *u_pBtn,
                  *d_ownBtn,
                  *s_epAddBtn,
                  *s_epRemBtn;

    QPixmap   a_ctive,
              d_isabled;

    KNFilterManager *f_ilManager;


  protected slots:
    void slotAddBtnClicked();
    void slotDelBtnClicked();
    void slotEditBtnClicked();
    void slotCopyBtnClicked();
    void slotUpBtnClicked();
    void slotDownBtnClicked();
    void slotSepAddBtnClicked();
    void slotSepRemBtnClicked();
    void slotItemSelectedFilter(int);
    void slotSelectionChangedFilter();
    void slotSelectionChangedMenu();

};


class XHeader {

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


class PostNewsTechnical : public Base {

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


class PostNewsTechnicalWidget : public BaseWidget {

  Q_OBJECT

  public:
    PostNewsTechnicalWidget(PostNewsTechnical *d, QWidget *p=0, const char *n=0);
    ~PostNewsTechnicalWidget();

    void load();
    void save();

  protected:
    QComboBox   *c_harset,
                *e_ncoding;
    QCheckBox   *u_seOwnCSCB,
                *g_enMIdCB,
                *i_ncUaCB;
    KNDialogListBox *l_box;
    QPushButton *a_ddBtn,
                *d_elBtn,
                *e_ditBtn;
    KLineEdit   *h_ost;
    QLabel      *h_ostL;

    PostNewsTechnical *d_ata;

  protected slots:
    void slotGenMIdCBToggled(bool b);
    void slotSelectionChanged();
    void slotItemSelected(int id);
    void slotAddBtnClicked();
    void slotDelBtnClicked();
    void slotEditBtnClicked();

};


class XHeaderConfDialog : public KDialogBase {

  public:
    XHeaderConfDialog(const QString &h=QString::null, QWidget *p=0, const char *n=0);
    ~XHeaderConfDialog();

    QString result();


  protected:
    KLineEdit *n_ame,
              *v_alue;

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


class PostNewsComposerWidget : public BaseWidget {

  Q_OBJECT

  public:
    PostNewsComposerWidget(PostNewsComposer *d, QWidget *p=0, const char *n=0);
    ~PostNewsComposerWidget();

    void load();
    void save();

  protected:
    KIntSpinBox *m_axLen;
    QCheckBox   *w_ordWrapCB,
                *o_wnSigCB,
                *r_ewrapCB,
                *a_uthSigCB,
                *c_ursorOnTopCB,
                *e_xternCB;
    KLineEdit   *i_ntro,
                *e_ditor;

    PostNewsComposer *d_ata;

  protected slots:
    void slotChooseEditor();

};


class PostNewsSpellingWidget : public BaseWidget {

  public:
    PostNewsSpellingWidget(QWidget *p=0, const char *n=0);
    ~PostNewsSpellingWidget();

    void save();

  protected:
     KSpellConfig *c_onf;

};



class PrivacyWidget : public BaseWidget {

  Q_OBJECT

  public:
    PrivacyWidget(QWidget *p=0, const char *n=0);
    ~PrivacyWidget();

    void load();
    void save();

  protected:
    Kpgp::Config *c_onf;
    QCheckBox *a_utoCheckSigCB;
};



//BEGIN: Cleanup configuration -----------------------------------------------

class Cleanup : public Base {

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


/** Configuration widget for group expireration */
class GroupCleanupWidget : public QWidget {

  Q_OBJECT

  public:
    GroupCleanupWidget( Cleanup *data, QWidget *parent = 0, const char *name = 0 );

    void load();
    void save();

  signals:
    void changed();

  private:
    QCheckBox *mDefault, *mExpEnabled, *mExpUnavailable, *mPreserveThreads;
    KIntSpinBox *mExpDays, *mExpReadDays, *mExpUnreadDays;
    QGroupBox *mExpGroup;
    Cleanup *mData;

  private slots:
    void slotDefaultToggled( bool state );

};


/** Global cleanup configuration widget */
class CleanupWidget : public BaseWidget {

  Q_OBJECT

  public:
    CleanupWidget(QWidget *p=0, const char *n=0);
    ~CleanupWidget();

    void load();
    void save();

  protected:
    QCheckBox   *f_olderCB;
    KIntSpinBox *f_olderDays;
    QLabel      *f_olderDaysL;

    Cleanup *d_ata;


  protected slots:
    void slotFolderCBtoggled(bool b);

  private:
    GroupCleanupWidget *mGroupCleanup;

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

};


class CacheWidget : public BaseWidget  {


  Q_OBJECT

  public:
    CacheWidget(Cache *d, QWidget *p=0, const char *n=0);
    ~CacheWidget();

    void apply();


  protected:
    KIntSpinBox *m_emMaxArt,
                *m_emMaxKB,
                *d_iskMaxArt,
                *d_iskMaxKB;

    QLabel      *d_iskMaxArtL,
                *d_iskMaxKBL;

    Cache *d_ata;


}; */


} //KNConfig

#endif //KNCONFIG_H
