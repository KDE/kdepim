/*
    knconfig.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
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

#include <kdialogbase.h>

#include "knwidgets.h"

class QRadioButton;
class QMultiLineEdit;
class QCheckBox;

class KScoringRule;
class KScoringEditorWidget;
class KpgpConfig;
class KConfigBase;
class KProcess;
class KLineEdit;
class KComboBox;
class KIntSpinBox;
class KSpellConfig;

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

    bool dirty()           { return d_irty; }
    void setDirty(bool b)  { d_irty=b; }

  protected:
    bool d_irty;

};


class BaseWidget : public QWidget {

  Q_OBJECT

  public:
    BaseWidget(QWidget *p=0, const char *n=0) : QWidget(p, n), d_irty(false) {}
    ~BaseWidget() {}

    void show()             { d_irty=true; QWidget::show(); }

    bool dirty()            { return d_irty; }
    void setDirty(bool b)   { d_irty=b; }

    virtual void apply() {}

  protected:
    bool d_irty;

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
    bool isGlobal()           { return g_lobal; }

    //personal information
    bool hasName()                    { return (!n_ame.isEmpty()); }
    QString name()                    { return n_ame; }
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
    QString orga()                    { return o_rga; }
    void setOrga(const QString &s)    { o_rga=s; }

    //signature
    bool hasSignature()       { return ( (u_seSigFile && !s_igPath.isEmpty()) || !s_igText.isEmpty() ); }
    bool useSigFile()         { return u_seSigFile; }
    bool useSigGenerator()    { return u_seSigGenerator; }
    QString sigPath()         { return s_igPath; }
    QString sigText()         { return s_igText; }
    QString getSignature();

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
              s_igPath;
    bool      u_seSigFile,
              u_seSigGenerator,
              g_lobal;
};


class IdentityWidget : public BaseWidget {

  Q_OBJECT

  public:
    IdentityWidget(Identity *d, QWidget *p=0, const char *n=0);
    ~IdentityWidget();

    void apply();


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
    QMultiLineEdit  *s_igEditor;
    QButtonGroup    *b_uttonGroup;

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
                *p_assLabel;
    KIntSpinBox *h_old,
                *t_imeout;
    QCheckBox   *f_etchDes,
                *a_uth,
                *u_seDiskCache;
    KNConfig::IdentityWidget* i_dWidget;

    KNNntpAccount *a_ccount;

  protected slots:
    void slotOk();
    void slotAuthChecked(bool b);

};


class SmtpAccountWidget : public BaseWidget {

Q_OBJECT

  public:
    SmtpAccountWidget(QWidget *p=0, const char *n=0);
    ~SmtpAccountWidget();

    void apply();

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

#define COL_CNT 13
#define FNT_CNT 5
#define HEX_CNT 4
#define ICON_CNT 18

  friend class AppearanceWidget;

  public:
    enum ColorIndex   { background=0, header=1, normalText=2, quoted1=3, quoted2=4,
                        quoted3=5, url=6, unreadThread=7, readThread=8, unreadArticle=9,
                        readArticle=10, activeItem=11, selectedItem=12 };

    enum HexIndex     { quoted1Hex=0, quoted2Hex=1, quoted3Hex=2, headerHex=3 };

    enum FontIndex    { article=0, articleFixed=1, composer=2, groupList=3, articleList=4 };

    enum IconIndex    { greyBall=0,        redBall=1,      greyBallChkd=2,
                        redBallChkd=3,     newFups=4,      eyes=5,
                        ignore=6,          mail=7,         posting=8,
                        canceledPosting=9, savedRemote=10, nntp=11,
                        group=12,          folder=13,      rootFolder=14,
                        customFolder=15,   sendErr=16,     null=17 };
    Appearance();
    ~Appearance();

    void save();

    QColor backgroundColor();
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
    QColor activeItemColor();
    QColor selectedItemColor();

    const char* headerDecoHexcode()       { return h_excodes[headerHex]; }
    const char* quotedTextHexcode(int i)  { return h_excodes[i]; }
    void updateHexcodes();

    bool useFontsForAllCS()              { return u_seFontsForAllCS; }

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
         u_seFonts,
         u_seFontsForAllCS;
    QColor  c_olors[COL_CNT];
    QString c_olorNames[COL_CNT];
    QFont   f_onts[FNT_CNT];
    QString f_ontNames[FNT_CNT];
    char    h_excodes[HEX_CNT][8];
    QPixmap i_cons[ICON_CNT];

};


class AppearanceWidget : public BaseWidget {

  Q_OBJECT

  public:
    AppearanceWidget(Appearance *d, QWidget *p=0, const char *n=0);
    ~AppearanceWidget();

    void apply();

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
              *f_ontCB,
              *f_ontCSCB;
    QPushButton *c_olDefBtn,
                *c_olChngBtn,
                *f_ntDefBtn,
                *f_ntChngBtn;

    Appearance *d_ata;

  protected slots:
    //colors
    void slotColCheckBoxToggled(bool b);
    void slotColItemSelected(QListBoxItem *);   // show color dialog for the entry
    void slotColDefaultBtnClicked();
    void slotColChangeBtnClicked();
    void slotColSelectionChanged();

    //fonts
    void slotFontCheckBoxToggled(bool b);
    void slotFontItemSelected(QListBoxItem *);  // show font dialog for the entry
    void slotFontDefaultBtnClicked();
    void slotFontChangeBtnClicked();
    void slotFontSelectionChanged();

};


class ReadNewsGeneral : public Base {

  friend class ReadNewsGeneralWidget;

  public:
    ReadNewsGeneral();
    ~ReadNewsGeneral();

    void save();

    bool autoCheckGroups()           { return a_utoCheck; }
    int maxToFetch()                 { return m_axFetch; }
    bool autoMark()                  { return a_utoMark; }
    int autoMarkSeconds()            { return m_arkSecs; }
    bool markCrossposts()            { return m_arkCrossposts; }

    bool smartScrolling()            { return s_martScrolling; }
    bool totalExpandThreads()        { return t_otalExpand; }
    bool defaultToExpandedThreads()  { return d_efaultExpand; }
    bool showLines()                 { return s_howLines; }
    bool showScore()                 { return s_howScore; }

    int collCacheSize()              { return c_ollCacheSize; }
    int artCacheSize()               { return a_rtCacheSize; }

    bool showThreads()               { return s_howThreads; }
    void setShowThreads(bool b)      { d_irty=true; s_howThreads=b;}

    bool autoCheckPgpSigs()          { return a_utoCheckPgpSigs; }
    void setAutoCheckPgpSigs(bool b) { d_irty=true; a_utoCheckPgpSigs=b;}

  protected:
    bool  a_utoCheck,
          a_utoMark,
          m_arkCrossposts,
          s_martScrolling,
          t_otalExpand,
          d_efaultExpand,
          s_howLines,
          s_howScore,
          s_howThreads,
          a_utoCheckPgpSigs;

    int   m_axFetch,
          m_arkSecs,
          c_ollCacheSize,
          a_rtCacheSize;

};


class ReadNewsGeneralWidget : public BaseWidget {

  public:
    ReadNewsGeneralWidget(ReadNewsGeneral *d, QWidget *p=0, const char *n=0);
    ~ReadNewsGeneralWidget();

    void apply();

  protected:
    QCheckBox   *a_utoCB,
                *m_arkCB,
                *m_arkCrossCB,
                *s_martScrollingCB,
                *e_xpThrCB,
                *d_efaultExpandCB,
                *l_inesCB,
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

    bool emulateKMail()               { return e_muKMail; }
    bool markAllReadGoNext()          { return m_arkAllReadGoNext; }
    bool markThreadReadGoNext()       { return m_arkThreadReadGoNext; }
    bool markThreadReadCloseThread()  { return m_arkThreadReadCloseThread; }
    bool ignoreThreadGoNext()         { return i_gnoreThreadGoNext; }
    bool ignoreThreadCloseThread()   { return i_gnoreThreadCloseThread; }

  protected:
    bool  e_muKMail,
          m_arkAllReadGoNext,
          m_arkThreadReadGoNext,
          m_arkThreadReadCloseThread,
          i_gnoreThreadGoNext,
          i_gnoreThreadCloseThread;

};


class ReadNewsNavigationWidget : public BaseWidget {

  public:
    ReadNewsNavigationWidget(ReadNewsNavigation *d, QWidget *p=0, const char *n=0);
    ~ReadNewsNavigationWidget();

    void apply();

  protected:
    QCheckBox   *e_muKMailCB,
                *m_arkAllReadGoNextCB,
                *m_arkThreadReadGoNextCB,
                *m_arkThreadReadCloseThreadCB,
                *i_gnoreThreadGoNextCB,
                *i_gnoreThreadCloseThreadCB;

    ReadNewsNavigation *d_ata;

};


class ReadNewsViewer : public Base {

  friend class ReadNewsViewerWidget;

  public:
    enum browserType { BTkonq=0, BTnetscape=1, BTmozilla=2, BTopera=3, BTother=4 };

    ReadNewsViewer();
    ~ReadNewsViewer();

    void save();

    bool showHeaderDecoration()      { return s_howHeaderDeco; }
    bool rewrapBody()                { return r_ewrapBody; }
    bool removeTrailingNewlines()    { return r_emoveTrailingNewlines; }
    bool showSignature()             { return s_howSig; }
    bool interpretFormatTags()       { return i_nterpretFormatTags; }
    QString quoteCharacters()        { return q_uoteCharacters; }

    bool showAttachmentsInline()     { return i_nlineAtt; }
    bool openAttachmentsOnClick()    { return o_penAtt; }
    bool showAlternativeContents()   { return s_howAlts; }

    browserType browser()            { return b_rowser; }
    QString browserCommand()         { return b_rowserCommand; }

    bool showFullHdrs()              { return f_ullHdrs; }
    void setShowFullHdrs(bool b)     { d_irty = true; f_ullHdrs=b; }
    bool useFixedFont()              { return u_seFixedFont; }
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

    void apply();

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
    typedef QListIterator<KNDisplayedHeader> Iterator;

    DisplayedHeaders();
    ~DisplayedHeaders();

    void save();

    KNDisplayedHeader* createNewHeader();
    void remove(KNDisplayedHeader *h);
    void up(KNDisplayedHeader *h);
    void down(KNDisplayedHeader *h);

    Iterator iterator()   { return Iterator(h_drList); }


  protected:
    QList<KNDisplayedHeader> h_drList;

};


class DisplayedHeadersWidget : public BaseWidget {

  Q_OBJECT

  public:
    DisplayedHeadersWidget(DisplayedHeaders *d, QWidget *p=0, const char *n=0);
    ~DisplayedHeadersWidget();

    void apply();

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

    void apply();

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

    void apply();

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

    QCString charset()          { return c_harset; }
    QStringList composerCharsets() { return c_omposerCharsets; }
    int indexForCharset(const QCString &str);
    QCString findComposerCharset(QCString cs);
    QCString findComposerCharset(QFont::CharSet cs);

    bool allow8BitBody()        { return a_llow8BitBody; }
    bool useOwnCharset()        { return u_seOwnCharset; }
    bool allow8BitHeaders()     { return (a_llow8BitHeaders && !o_verrideAllow8BitHeaders); }
    void disableAllow8BitHeaders(bool b)  { o_verrideAllow8BitHeaders = b; }
    bool generateMessageID()    { return g_enerateMID; }
    QCString hostname()         { return h_ostname; }
    XHeaders& xHeaders()        { return x_headers; }
    bool noUserAgent()          { return d_ontIncludeUA; }
    bool useExternalMailer()    { return u_seExternalMailer; }

  protected:
    QCString  c_harset,
              h_ostname;
    QStringList c_omposerCharsets;

    bool      a_llow8BitBody,
              u_seOwnCharset,
              a_llow8BitHeaders,
              o_verrideAllow8BitHeaders,
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

    void apply();

  protected:
    QComboBox   *c_harset,
                *e_ncoding;
    QCheckBox   *u_seOwnCSCB,
                *a_llow8bitCB,
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
    void slotHeadEncToggled(bool b);
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

    bool wordWrap()             { return w_ordWrap; }
    int maxLineLength()         { return m_axLen; }
    bool appendOwnSignature()   { return a_ppSig; }

    QString intro()             { return i_ntro; }
    bool rewrap()               { return r_ewrap; }
    bool includeSignature()     { return i_ncSig; }
    bool cursorOnTop()          { return c_ursorOnTop; }

    QString externalEditor()    { return e_xternalEditor; }
    bool useExternalEditor()    { return u_seExtEditor; }


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

    void apply();

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

    void apply();

  protected:
     KSpellConfig *c_onf;

};



class PrivacyWidget : public BaseWidget {

  Q_OBJECT

  public:
    PrivacyWidget(QWidget *p=0, const char *n=0);
    ~PrivacyWidget();
    void apply();   // overrides BaseWidget::apply()

  protected:
    KpgpConfig *c_onf;
    QCheckBox *a_utoCheckSigCB;
};


class Cleanup : public Base {

  friend class CleanupWidget;

  public:
    Cleanup();
    ~Cleanup();

    void save();

    //expire
    int maxAgeForRead()         { return r_eadMaxAge; }
    int maxAgeForUnread()       { return u_nreadMaxAge; }
    bool removeUnavailable()    { return r_emoveUnavailable; }
    bool preserveThreads()      { return p_reserveThr; }
    bool expireToday();
    void setLastExpireDate();

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

};


class CleanupWidget : public BaseWidget {

  Q_OBJECT

  public:
    CleanupWidget(Cleanup *d, QWidget *p=0, const char *n=0);
    ~CleanupWidget();

    void apply();

  protected:
    QCheckBox   *f_olderCB,
                *g_roupCB,
                *u_navailableCB,
                *t_hrCB;
    KIntSpinBox *f_olderDays,
                *g_roupDays,
                *r_eadDays,
                *u_nreadDays;
    QLabel      *f_olderDaysL,
                *g_roupDaysL,
                *r_eadDaysL,
                *u_nreadDaysL;

    Cleanup *d_ata;


  protected slots:
    void slotGroupCBtoggled(bool b);
    void slotFolderCBtoggled(bool b);

};


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


}; //KNConfig

#endif //KNCONFIG_H
