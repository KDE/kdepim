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

#include <qlabel.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qpixmap.h>
#include <qlist.h>
#include <qstringlist.h>
#include <qasciidict.h>

#include <knuminput.h>
#include <kspell.h>
#include <kdialogbase.h>
#include <kconfigbase.h>

#include "knlistbox.h"

class KNNntpAccount;
class KNAccountManager;
class KNArticleFilter;
class KNFilterManager;
class KNDisplayedHeader;
class KNServerInfo;


namespace KNConfig {

class Base {

  public:
    Base()    {}
    virtual ~Base()   {}

    virtual void save() {}

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
    QLineEdit       *n_ame,
                    *o_rga,
                    *e_mail,
                    *r_eplyTo,
                    *s_ig;
    QRadioButton    *s_igFile,
                    *s_igEdit;
    QCheckBox       *s_igGenerator;
    QPushButton     *c_hooseBtn,
                    *e_ditBtn;
    QMultiLineEdit  *s_igEditor;

    Identity        *d_ata;

  protected slots:
    void slotSignatureType(int type);
    void slotSignatureChoose();
    void slotSignatureEdit();

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
    QLineEdit   *n_ame,
                *s_erver,
                *u_ser,
                *p_ass,
                *p_ort;
    QLabel      *u_serLabel,
                *p_assLabel;
    KIntSpinBox *h_old,
                *t_imeout;
    QCheckBox   *f_etchDes,
                *a_uth;
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
    void useKmailToggled(bool b);

  protected:
    KNServerInfo  *s_erverInfo;
    QCheckBox     *u_seKmail;
    QLineEdit     *s_erver,
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
#define FNT_CNT 4
#define HEX_CNT 4
#define ICON_CNT 14

  friend class AppearanceWidget;

  public:
    enum ColorIndex   { background=0, header=1, normalText=2, quoted1=3, quoted2=4,
                        quoted3=5, url=6, unreadThread=7, readThread=8, unreadArticle=9,
                        readArticle=10, activeItem=11, selectedItem=12 };

    enum HexIndex     { quoted1Hex=0, quoted2Hex=1, quoted3Hex=2, headerHex=3 };

    enum FontIndex    { article=0, composer=1, groupList=2, articleList=3 };

    enum IconIndex    { greyBall=0,       redBall=1,      greyBallChkd=2,
                        redBallChkd=3,    newFups=4,      eyes=5,
                        mail=6,           posting=7,      canceledPosting=8,
                        nntp=9,           group=10,       folder=11,
                        sendErr=12,       null=13 };
    Appearance();
    ~Appearance();

    void save();

    bool longGroupList()              { return l_ongGroupList; }

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

    bool l_ongGroupList,
         u_seColors,
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
    QCheckBox *l_ongCB,
              *c_olorCB,
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

    bool totalExpandThreads()        { return t_otalExpand; }
    bool showLines()                 { return s_howLines; }
    bool showScore()                 { return s_howScore; }

  protected:
    bool  a_utoCheck,
          a_utoMark,
          t_otalExpand,
          s_howLines,
          s_howScore;

    int   m_axFetch,
          m_arkSecs;

};


class ReadNewsGeneralWidget : public BaseWidget {

  public:
    ReadNewsGeneralWidget(ReadNewsGeneral *d, QWidget *p=0, const char *n=0);
    ~ReadNewsGeneralWidget();

    void apply();

  protected:
    QCheckBox   *a_utoCB,
                *m_arkCB,
                *e_xpThrCB,
                *l_inesCB,
                *s_coreCB;
    KIntSpinBox *m_arkSecs,
                *m_axFetch;

    ReadNewsGeneral *d_ata;

};


class ReadNewsViewer : public Base {

  friend class ReadNewsViewerWidget;

  public:
    enum browserType { BTkonq=0, BTnetscape=1, BTmozilla=2, BTopera=3, BTother=4 };

    ReadNewsViewer();
    ~ReadNewsViewer();

    void save();

    bool showSignature()             { return s_howSig; }
    bool interpretFormatTags()       { return i_nterpretFormatTags; }

    bool showAttachmentsInline()     { return i_nlineAtt; }
    bool openAttachmentsOnClick()    { return o_penAtt; }
    bool showAlternativeContents()   { return s_howAlts; }

    browserType browser()            { return b_rowser; }
    QString browserCommand()         { return b_rowserCommand; }

  protected:
    bool  s_howSig,
          i_nterpretFormatTags,
          i_nlineAtt,
          o_penAtt,
          s_howAlts;

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
    QCheckBox   *s_igCB,
                *i_nlineCB,
                *o_penAttCB,
                *a_ltAttCB,
                *f_ormatCB;
    QComboBox   *b_rowser;
    QLineEdit   *b_rowserCommand;
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
    QComboBox *h_drC;
    QLineEdit *n_ameE;
    QCheckBox *n_ameCB[4],
              *v_alueCB[4];


  protected slots:
    void slotOk();
    void slotActivated(int);
    void slotNameChanged(const QString&);
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
    bool allow8BitHeaders()     { return a_llow8BitHeaders; }
    bool generateMessageID()    { return g_enerateMID; }
    QCString hostname()         { return h_ostname; }
    XHeaders& xHeaders()        { return x_headers; }
    bool noUserAgent()          { return d_ontIncludeUA; }
    bool useKmail()             { return u_seKmail; }

  protected:
    QCString  c_harset,
              h_ostname;
    QStringList c_omposerCharsets;

    bool      a_llow8BitBody,
              u_seOwnCharset,
              a_llow8BitHeaders,
              g_enerateMID,
              d_ontIncludeUA,
              u_seKmail;

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
    QLineEdit   *h_ost;
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
    QLineEdit *n_ame,
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

    QString externalEditor()    { return e_xternalEditor; }
    bool useExternalEditor()    { return u_seExtEditor; }


  protected:
    int     m_axLen;
    bool    w_ordWrap,
            a_ppSig,
            r_ewrap,
            i_ncSig,
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
                *a_uthSigCB,
                *r_ewrapCB,
                *e_xternCB;
    QLineEdit   *i_ntro,
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


class Privacy: public Base {

  friend class PrivacyWidget;
 
  public:
    Privacy();
    ~Privacy();
    void save();

    enum { GPG1 = 0, PGP2, PGP5, PGP6 };
    QString defaultProg(int version);

  private:
    // k_eyserv is the used Keyserver
    QString k_eyserv;
    // p_rogpath is the path to the used PGP program, ie /usr/bin/gpg
    QString p_rogpath;
    // k_eyring is the used Keyring
    QString k_eyring;
    // v_ersions contains a list of currently supported PGP/GPG versions
    const QStringList v_ersions;
    // e_ncodings contains a list of currently supported message encodings
    // (that means something like where to store the signature in an article)
    const QStringList e_ncodings;
    // v_version is the used version, its in fact the offset in the v_ersions list
    int v_ersion;
    // e_ncoding is the used encoding, its in fact the offset in the e_ncodings list
    int e_ncoding;
    // shall knode store the passphrase in memory? (thats an security hole, but really handy)
    bool k_eeppasswd;

};


class PrivacyWidget : public BaseWidget {

  Q_OBJECT
 
  public:
    PrivacyWidget(Privacy *p, QWidget *_p=0, const char *n=0);
    ~PrivacyWidget();
    void apply();   // overrides BaseWidget::apply()
 
  protected:
 
    Privacy* d_ata;
 
    QComboBox *pgp_version,
              *enc_style;
    QCheckBox *keeppasswd;
    QLineEdit *keyring,
              *keyserv,
              *pgp_path;
  protected slots:
    void slotDefaultProg(int);
    void slotChoosePgp();

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
    bool preserveThreads()      { return p_reserveThr; }
    bool expireToday();
    void setLastExpireDate();

    //compact
    bool compactToday();
    void setLastCompactDate();


  protected:
    bool  d_oExpire,
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

}; //KNConfig

#endif //KNCONFIG_H
