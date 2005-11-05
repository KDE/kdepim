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
#include "nntpaccountdialog_base.h"
#include "postnewstechnicalwidget_base.h"
#include "readnewsgeneralwidget_base.h"
#include "smtpaccountwidget_base.h"

//Added by qt3to4:
#include <QPixmap>
#include <QLabel>
#include <Q3ValueList>

class Q3ButtonGroup;
class QCheckBox;
class Q3GroupBox;
class QRadioButton;
class Q3TextEdit;

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

namespace KNode {
  class Appearance;
  class Cleanup;
  class Identity;
  class DisplayedHeaders;
  class GroupCleanupWidget;
  class PostNewsTechnical;
  class Scoring;
}

namespace KNode {

/** Configuration widget for an dentity.
 */
class KDE_EXPORT IdentityWidget : public KCModule {

  Q_OBJECT

  public:
    IdentityWidget( Identity *d, KInstance *inst, QWidget *parent = 0 );
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
    Q3TextEdit       *s_igEditor;
    Q3ButtonGroup    *b_uttonGroup;
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


/** News server account list widget. */
class KDE_EXPORT NntpAccountListWidget : public KCModule {

  Q_OBJECT

  public:
    NntpAccountListWidget( KInstance *inst, QWidget *parent = 0 );
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


/** News server configuration dialog. */
class KDE_EXPORT NntpAccountConfDialog : public KDialogBase, private Ui::NntpAccountDialogBase  {

  Q_OBJECT

  public:
    NntpAccountConfDialog( KNNntpAccount *a, QWidget *parent = 0 );
    ~NntpAccountConfDialog();

  protected slots:
    virtual void slotOk();

  private slots:
    void slotPasswordChanged();

  private:
    KNNntpAccount *mAccount;
    IdentityWidget* mIdentityWidget;
    GroupCleanupWidget *mCleanupWidget;
};


/** Configuration dialog for the SMTP account */
class KDE_EXPORT SmtpAccountWidget : public KCModule, private Ui::SmtpAccountWidgetBase {

Q_OBJECT

  public:
    SmtpAccountWidget( KInstance *inst, QWidget *parent = 0 );
    ~SmtpAccountWidget() {}

    virtual void load();
    virtual void save();

  protected slots:
    void useExternalMailerToggled( bool b );
    void loginToggled( bool b );
    void slotPasswordChanged();

  protected:
    KNServerInfo  *mAccount;
};


/** Appearance configuration widget. */
class KDE_EXPORT AppearanceWidget : public KCModule {

  Q_OBJECT

  public:
    AppearanceWidget( KInstance *inst,QWidget *parent = 0 );
    ~AppearanceWidget();

    void load();
    void save();
    void defaults();

    //===================================================================================
    // code taken from KMail, Copyright (C) 2000 Espen Sand, espen@kde.org

    class KDE_EXPORT ColorListItem : public Q3ListBoxText {

      public:
        ColorListItem( const QString &text, const QColor &color=Qt::black );
        ~ColorListItem();
        const QColor& color()                     { return mColor; }
        void  setColor( const QColor &color )     { mColor = color; }

      protected:
        virtual void paint( QPainter * );
        virtual int height( const Q3ListBox * ) const;
        virtual int width( const Q3ListBox * ) const;

      private:
        QColor mColor;
    };

    //===================================================================================

    class KDE_EXPORT FontListItem : public Q3ListBoxText {

      public:
        FontListItem( const QString &name, const QFont & );
        ~FontListItem();
        const QFont& font()                     { return f_ont; }
        void setFont( const QFont &);

      protected:
        virtual void paint( QPainter * );
        virtual int width( const Q3ListBox * ) const;

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
    void slotColItemSelected(Q3ListBoxItem *);   // show color dialog for the entry
    void slotColChangeBtnClicked();
    void slotColSelectionChanged();

    //fonts
    void slotFontCheckBoxToggled(bool b);
    void slotFontItemSelected(Q3ListBoxItem *);  // show font dialog for the entry
    void slotFontChangeBtnClicked();
    void slotFontSelectionChanged();

};


/** General read news configuration page.
 * @todo Use KConfigXT also for handling the date format button group.
 */
class KDE_EXPORT ReadNewsGeneralWidget : public KCModule, KNode::Ui::ReadNewsGeneralWidgetBase
{
  public:
    /** Create a new general configuration page.
     * @param parent The QWidget parent.
     */
    ReadNewsGeneralWidget( KInstance *inst, QWidget *parent = 0 );

    virtual void load();
    virtual void save();
};


/** Read news navigation configuration page. */
class KDE_EXPORT ReadNewsNavigationWidget : public KCModule
{
  public:
    /** Create a new navigation configuration page.
     * @param parent The QWidget parent.
     */
    ReadNewsNavigationWidget( KInstance *inst, QWidget *parent = 0 );
};


/** Article viewer configuration page. */
class KDE_EXPORT ReadNewsViewerWidget : public KCModule
{
  public:
    /** Create a new article viewer configuration page.
     * @param parent The QWidget parent.
     */
    ReadNewsViewerWidget( KInstance *inst, QWidget *parent = 0 );
};


/** Configuration widget for headers displayed in the article viewer. */
class KDE_EXPORT DisplayedHeadersWidget : public KCModule {

  Q_OBJECT

  public:
    DisplayedHeadersWidget( DisplayedHeaders *d, KInstance *inst, QWidget *parent = 0 );
    ~DisplayedHeadersWidget();

    void load();
    void save();

  protected:

     class HdrItem : public Q3ListBoxText {

      public:
        HdrItem( const QString &t, KNDisplayedHeader *h ) : Q3ListBoxText(t), hdr(h) {}
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


/** Configuration dialog for a single header displayed in the article viewer. */
class KDE_EXPORT DisplayedHeaderConfDialog : public KDialogBase {

  Q_OBJECT

  public:
    DisplayedHeaderConfDialog( KNDisplayedHeader *h, QWidget *parent = 0 );
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


/** Scoring configuration widget. */
class KDE_EXPORT ScoringWidget : public KCModule
{
  public:
    /** Create a new scoring configuration widget.
     * @param parent The QWidget parent.
     */
    ScoringWidget( KInstance *inst, QWidget *parent = 0 );

    virtual void load();
    virtual void save();

  private:
    KScoringEditorWidget *mKsc;
    KIntSpinBox *mIgnored, *mWatched;
};


/** Configuration widget for filters. */
class KDE_EXPORT FilterListWidget : public KCModule {

  Q_OBJECT

  public:
    FilterListWidget( KInstance *inst, QWidget *parent = 0 );
    ~FilterListWidget();

    void load();
    void save();

    void addItem(KNArticleFilter *f);
    void removeItem(KNArticleFilter *f);
    void updateItem(KNArticleFilter *f);
    void addMenuItem(KNArticleFilter *f);
    void removeMenuItem(KNArticleFilter *f);
    Q3ValueList<int> menuOrder();


  protected:
    class LBoxItem : public KNListBoxItem {
      public:
        LBoxItem(KNArticleFilter *f, const QString &t, QPixmap *p=0)
          : KNListBoxItem(t, p) , filter(f) {}
        ~LBoxItem() {}

        KNArticleFilter *filter;
    };

    int findItem(Q3ListBox *l, KNArticleFilter *f);

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


/** Configuration widget for technical posting settings. */
class KDE_EXPORT PostNewsTechnicalWidget : public KCModule, KNode::Ui::PostNewsTechnicalWidgetBase
{
  Q_OBJECT
  public:
    PostNewsTechnicalWidget( PostNewsTechnical *d, KInstance *inst, QWidget *parent = 0 );

    void load();
    void save();

  protected:
    PostNewsTechnical *mData;

  private slots:
    void slotSelectionChanged();
    void slotAddBtnClicked();
    void slotDelBtnClicked();
    void slotEditBtnClicked();

};


/** Dialog to edit additional headers. */
class KDE_EXPORT XHeaderConfDialog : public KDialogBase
{
  public:
    /** Create a new dialog to edit an additional header.
     * @param h The header to edit.
     * @param parent The parent widget.
     */
    XHeaderConfDialog( const QString &h = QString::null, QWidget *parent = 0 );
    /** Destructor. */
    ~XHeaderConfDialog();

    /** Returns the entered/modified header. */
    QString result() const;

  private:
    KLineEdit *mNameEdit, *mValueEdit;
};


/** Composer configuration widget. */
class KDE_EXPORT PostNewsComposerWidget : public KCModule
{
  public:
    /** Create a new composer configuration widget.
     * @param parent The parent widget.
     */
    PostNewsComposerWidget( KInstance *inst, QWidget *parent = 0 );
};


/** Spell-checking configuration widget. */
class KDE_EXPORT PostNewsSpellingWidget : public KCModule {

  public:
    PostNewsSpellingWidget( KInstance *inst, QWidget *parent = 0 );
    ~PostNewsSpellingWidget();

    void save();

  protected:
     KSpellConfig *c_onf;

};


/** Privacy configuration widget. */
class KDE_EXPORT PrivacyWidget : public KCModule {

  Q_OBJECT

  public:
    PrivacyWidget( KInstance *inst, QWidget *parent = 0 );
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
    GroupCleanupWidget( Cleanup *data, QWidget *parent = 0 );

    void load();
    void save();

  signals:
    void changed();

  private:
    QCheckBox *mDefault, *mExpEnabled, *mExpUnavailable, *mPreserveThreads;
    KIntSpinBox *mExpDays, *mExpReadDays, *mExpUnreadDays;
    Q3GroupBox *mExpGroup;
    Cleanup *mData;

  private slots:
    void slotDefaultToggled( bool state );

};


/** Global cleanup configuration widget */
class KDE_EXPORT CleanupWidget : public KCModule {

  Q_OBJECT

  public:
    CleanupWidget( KInstance *inst, QWidget *parent = 0 );
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

} //KNode

#endif //KNCONFIGWIDGETS_H
