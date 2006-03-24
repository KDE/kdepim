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

#ifndef KNCONFIGWIDGETS_H
#define KNCONFIGWIDGETS_H

#include <kdialogbase.h>
#include <kcmodule.h>

#include "knwidgets.h"
#include "smtpaccountwidget_base.h"

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QRadioButton;
class QTextEdit;

class KScoringEditorWidget;
class KConfigBase;
class KLineEdit;
class KComboBox;
class KIntSpinBox;
class KSpellConfig;
class KURLCompletion;

namespace Kpgp {
  class Config;
  class SecretKeyRequester;
}

class KNAccountManager;
class KNArticleFilter;
class KNDisplayedHeader;
class KNFilterManager;
class KNNntpAccount;
class KNServerInfo;

namespace KNConfig {
  class Appearance;
  class Cleanup;
  class Identity;
  class DisplayedHeaders;
  class GroupCleanupWidget;
  class PostNewsTechnical;
  class ReadNewsGeneral;
  class ReadNewsNavigation;
  class PostNewsComposer;
  class ReadNewsViewer;
  class Scoring;
}

namespace KNConfig {

class KDE_EXPORT IdentityWidget : public KCModule {

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


class KDE_EXPORT NntpAccountListWidget : public KCModule {

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


class KDE_EXPORT NntpAccountConfDialog : public KDialogBase  {

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

  private slots:
    void slotPasswordChanged();

  private:
    GroupCleanupWidget *mCleanupWidget;
};


class KDE_EXPORT SmtpAccountWidget : public SmtpAccountWidgetBase {

Q_OBJECT

  public:
    SmtpAccountWidget(QWidget *p=0, const char *n=0);
    ~SmtpAccountWidget() {}

    virtual void load();
    virtual void save();

  protected slots:
    virtual void useExternalMailerToggled( bool b );
    virtual void loginToggled( bool b );
    void slotPasswordChanged();

  protected:
    KNServerInfo  *mAccount;
};


class KDE_EXPORT AppearanceWidget : public KCModule {

  Q_OBJECT

  public:
    AppearanceWidget(QWidget *p=0, const char *n=0);
    ~AppearanceWidget();

    void load();
    void save();
    void defaults();

    //===================================================================================
    // code taken from KMail, Copyright (C) 2000 Espen Sand, espen@kde.org

    class KDE_EXPORT ColorListItem : public QListBoxText {

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

    class KDE_EXPORT FontListItem : public QListBoxText {

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


class KDE_EXPORT ReadNewsGeneralWidget : public KCModule {

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


class KDE_EXPORT ReadNewsNavigationWidget : public KCModule {

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


class KDE_EXPORT ReadNewsViewerWidget : public KCModule {

  Q_OBJECT

  public:
    ReadNewsViewerWidget(ReadNewsViewer *d, QWidget *p=0, const char *n=0);
    ~ReadNewsViewerWidget();

    void load();
    void save();

  protected:
    QCheckBox   *r_ewrapCB,
                *r_emoveTrailingCB,
                *s_igCB,
                *o_penAttCB,
                *a_ltAttCB,
                *mShowRefBar,
                *mAlwaysShowHTML;
    KLineEdit *q_uoteCharacters;

    ReadNewsViewer *d_ata;

};


class KDE_EXPORT DisplayedHeadersWidget : public KCModule {

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


class KDE_EXPORT DisplayedHeaderConfDialog : public KDialogBase {

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


class KDE_EXPORT ScoringWidget : public KCModule {

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


class KDE_EXPORT FilterListWidget : public KCModule {

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


class KDE_EXPORT PostNewsTechnicalWidget : public KCModule {

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


class KDE_EXPORT XHeaderConfDialog : public KDialogBase {

  public:
    XHeaderConfDialog(const QString &h=QString::null, QWidget *p=0, const char *n=0);
    ~XHeaderConfDialog();

    QString result();


  protected:
    KLineEdit *n_ame,
              *v_alue;

};


class KDE_EXPORT PostNewsComposerWidget : public KCModule {

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


class KDE_EXPORT PostNewsSpellingWidget : public KCModule {

  public:
    PostNewsSpellingWidget(QWidget *p=0, const char *n=0);
    ~PostNewsSpellingWidget();

    void save();

  protected:
     KSpellConfig *c_onf;

};



class KDE_EXPORT PrivacyWidget : public KCModule {

  Q_OBJECT

  public:
    PrivacyWidget(QWidget *p=0, const char *n=0);
    ~PrivacyWidget();

    void save();

  protected:
    Kpgp::Config *c_onf;
};



//BEGIN: Cleanup configuration -----------------------------------------------

/** Configuration widget for group expireration */
class KDE_EXPORT GroupCleanupWidget : public QWidget {

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
    void expDaysChanged( int value );
    void expReadDaysChanged( int value );
    void expUnreadDaysChanged( int value );
};


/** Global cleanup configuration widget */
class KDE_EXPORT CleanupWidget : public KCModule {

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
    void slotFolderDaysChanged(int value);

  private:
    GroupCleanupWidget *mGroupCleanup;

};

//END: Cleanup configuration -------------------------------------------------


/*class CacheWidget : public KCModule  {


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

#endif //KNCONFIGWIDGETS_H
