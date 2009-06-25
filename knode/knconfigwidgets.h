/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2006 the KNode authors.
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

#include "knode_export.h"
#include <kpagedialog.h>
#include <kcmodule.h>

#include "ui_nntpaccountdialog_base.h"
#include "ui_nntpaccountlistwidget_base.h"
#include "ui_postnewstechnicalwidget_base.h"
#include "ui_readnewsgeneralwidget_base.h"

#include <QList>
#include <QPixmap>
#include <QLabel>
#include <QListWidgetItem>

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QRadioButton;
class QTextEdit;
class QListWidget;

class KLineEdit;
class KComboBox;
class KIntSpinBox;
class KUrlCompletion;

namespace Sonnet{
class ConfigWidget;
}

namespace KPIM {
  class KScoringEditorWidget;
}

namespace Kpgp {
  class Config;
  class SecretKeyRequester;
}

class KNArticleFilter;
class KNDisplayedHeader;
class KNFilterManager;
class KNNntpAccount;

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
class KNODE_EXPORT IdentityWidget : public KCModule {

  Q_OBJECT

  public:
    IdentityWidget( Identity *d, const KComponentData &inst, QWidget *parent = 0 );
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
    KUrlCompletion  *c_ompletion;

    Identity        *d_ata;

  protected slots:
    void slotSignatureType(int type);
    void slotSignatureChoose();
    void slotSignatureEdit();
    void textFileNameChanged(const QString &);

};


/** News server account list widget. */
class KNODE_EXPORT NntpAccountListWidget : public KCModule, private Ui::NntpAccountListWidgetBase
{
  Q_OBJECT

  public:
    /** Create a new NNTP account list widget. */
    NntpAccountListWidget( const KComponentData &inst, QWidget *parent = 0 );

    /** Reimplemented from KCModule. */
    virtual void load();

  protected:
    /** Account list widget item. */
    class AccountListItem : public QListWidgetItem {
      public:
        /** Creates a new account list item.
         * @param a The account.
         */
        AccountListItem( KNNntpAccount *a ) :  mAccount( a ) {}
        /** Returns the account assiciated with this item. */
        KNNntpAccount *account() const { return mAccount; }
      private:
        KNNntpAccount *mAccount;
    };

  public slots:
    /** Add an list view item for the given account.
     * @param a The new account.
     */
    void slotAddItem( KNNntpAccount *a );
    /** Remove the list view item of the given account.
     * @param a The account.
     */
    void slotRemoveItem( KNNntpAccount *a );
    /** Update the item of the given account.
     * @param a The account.
     */
    void slotUpdateItem(KNNntpAccount *a);

  protected slots:
    /** Item selection has changed. */
    void slotSelectionChanged();
    /** Add account button has been clicked. */
    void slotAddBtnClicked();
    /** Delete account button has been clicked. */
    void slotDelBtnClicked();
    /** Edit account button has been clicked. */
    void slotEditBtnClicked();
    /** Subscribe button has been clicked. */
    void slotSubBtnClicked();

};


/** News server configuration dialog. */
class KNODE_EXPORT NntpAccountConfDialog : public KPageDialog, private Ui::NntpAccountDialogBase
{
  Q_OBJECT

  public:
    NntpAccountConfDialog( KNNntpAccount *a, QWidget *parent = 0 );
    ~NntpAccountConfDialog();

  protected slots:
    void slotServerTextEdited();
    void slotEditingFinished();
    virtual void slotButtonClicked( int button );

  private slots:
    void slotPasswordChanged();
    /**
      Changes the port to follow change of the encryption selection.
    */
    void encryptionChanged( bool checked );

  private:
    KNNntpAccount *mAccount;
    IdentityWidget* mIdentityWidget;
    GroupCleanupWidget *mCleanupWidget;
    bool mUseServerForName;
};


/** Appearance configuration widget. */
class KNODE_EXPORT AppearanceWidget : public KCModule
{
  Q_OBJECT

  public:
    /** Create a new appearance configuration widget.
     * @param inst The KComponentData.
     * @param parent The parent widget.
     */
    AppearanceWidget( const KComponentData &inst, QWidget *parent = 0 );

    /** Reimplemented from KCModule. */
    virtual void load();
    /** Reimplemented from KCModule. */
    virtual void save();
    /** Reimplemented from KCModule. */
    virtual void defaults();

    //===================================================================================

    /** Color list view item. */
    class KNODE_EXPORT ColorListItem : public QListWidgetItem {

      public:
        /** Create a new color list view item.
         * @param text The item text.
         * @param color The item color.
         * @param parent The list widget this item is shown in.
         */
        ColorListItem( const QString &text, const QColor &color = Qt::black, QListWidget *parent = 0 );
        /** Returns the current color. */
        const QColor& color() { return mColor; }
        /** Sets the current color. */
        void  setColor( const QColor &color );

      private:
        /// The current color.
        QColor mColor;
    };

    //===================================================================================

    /** Font list view item. */
    class KNODE_EXPORT FontListItem : public QListWidgetItem {

      public:
        /** Create a new font list view item.
         * @param text The item text.
         * @param font The selected font for this item.
         * @param parent The list widget this item is shown in.
         */
        FontListItem( const QString &text, const QFont &font, QListWidget *parent = 0 );
        /** Returns the current font. */
        const QFont& font() { return mFont; }
        /** Sets the current font.
         * @param font The new font.
         */
        void setFont( const QFont &font );

      private:
        /// The current font.
        QFont mFont;
        /// The item text.
        QString mText;
    };

    //===================================================================================

  protected:
    /// The color selection list.
    QListWidget *mColorList;
    /// The font selection list.
    QListWidget *mFontList;
    QCheckBox *c_olorCB,
              *f_ontCB;
    QPushButton *c_olChngBtn,
                *f_ntChngBtn;

  protected slots:
    //colors
    void slotColCheckBoxToggled(bool b);
    /** Show color selection dialog for the given item.
     * @param item The color list item that has been activated.
     */
    void slotColItemActivated( QListWidgetItem *item );
    void slotColChangeBtnClicked();
    void slotColSelectionChanged();

    //fonts
    void slotFontCheckBoxToggled(bool b);
    /** Show font selection dialog for the given item.
     * @param item The font list item that has been activated.
     */
    void slotFontItemActivated( QListWidgetItem *item );
    void slotFontChangeBtnClicked();
    void slotFontSelectionChanged();

};


/** General read news configuration page.
 * @todo Use KConfigXT also for handling the date format button group.
 */
class KNODE_EXPORT ReadNewsGeneralWidget : public KCModule, KNode::Ui::ReadNewsGeneralWidgetBase
{
  public:
    /** Create a new general configuration page.
     * @param parent The QWidget parent.
     */
    ReadNewsGeneralWidget( const KComponentData &inst, QWidget *parent = 0 );

    /** Reimplemented from KCModule. */
    virtual void load();
    /** Reimplemented from KCModule. */
    virtual void save();
};


/** Read news navigation configuration page. */
class KNODE_EXPORT ReadNewsNavigationWidget : public KCModule
{
  public:
    /** Create a new navigation configuration page.
     * @param parent The QWidget parent.
     */
    ReadNewsNavigationWidget( const KComponentData &inst, QWidget *parent = 0 );
};


/** Article viewer configuration page. */
class KNODE_EXPORT ReadNewsViewerWidget : public KCModule
{
  public:
    /** Create a new article viewer configuration page.
     * @param parent The QWidget parent.
     */
    ReadNewsViewerWidget( const KComponentData &inst, QWidget *parent = 0 );
};


/** Configuration widget for headers displayed in the article viewer. */
class KNODE_EXPORT DisplayedHeadersWidget : public KCModule {

  Q_OBJECT

  public:
    DisplayedHeadersWidget( DisplayedHeaders *d, const KComponentData &inst, QWidget *parent = 0 );

    /** Reimplemented from KCModule. */
    virtual void load();
    /** Reimplemented from KCModule. */
    virtual void save();

  protected:

    /** Header list view item. */
    class HdrItem : public QListWidgetItem {

      public:
        /** Creates a new header list view item.
         * @param text The text to display.
         * @param header The associated header.
         */
        HdrItem( const QString &text, KNDisplayedHeader *header ) : QListWidgetItem( text ), mHdr( header ) {}
        /** Returns the associated header. */
        KNDisplayedHeader *header() const { return mHdr; }
      private:
        KNDisplayedHeader *mHdr;
    };

    HdrItem* generateItem(KNDisplayedHeader *);

    QListWidget *mHeaderList;
    QPushButton *a_ddBtn,
                *d_elBtn,
                *e_ditBtn,
                *u_pBtn,
                *d_ownBtn;
    bool s_ave;

    DisplayedHeaders *d_ata;

  protected slots:
    void slotSelectionChanged();
    void slotAddBtnClicked();
    void slotDelBtnClicked();
    void slotEditBtnClicked();
    void slotUpBtnClicked();
    void slotDownBtnClicked();

};


/** Configuration dialog for a single header displayed in the article viewer. */
class KNODE_EXPORT DisplayedHeaderConfDialog : public KDialog {

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
class KNODE_EXPORT ScoringWidget : public KCModule
{
  public:
    /** Create a new scoring configuration widget.
     * @param parent The QWidget parent.
     */
    ScoringWidget( const KComponentData &inst, QWidget *parent = 0 );

  private:
    KPIM::KScoringEditorWidget *mKsc;
    KIntSpinBox *mIgnored, *mWatched;
};


/** Configuration widget for filters. */
class KNODE_EXPORT FilterListWidget : public KCModule {

  Q_OBJECT

  public:
    FilterListWidget( const KComponentData &inst, QWidget *parent = 0 );
    ~FilterListWidget();

    void load();
    void save();

    void addItem(KNArticleFilter *f);
    void removeItem(KNArticleFilter *f);
    void updateItem(KNArticleFilter *f);
    void addMenuItem(KNArticleFilter *f);
    void removeMenuItem(KNArticleFilter *f);
    QList<int> menuOrder();


  protected:
    /** Filter list view item. */
    class FilterListItem : public QListWidgetItem {
      public:
        /** Create new filter list item.
         * @param filter The associated filter object.
         * @param text The text to display in the item.
         * */
        FilterListItem( KNArticleFilter *filter, const QString &text ) : QListWidgetItem( text ), mFilter( filter ) {}
        /** Returns the associated filter. */
        KNArticleFilter* filter() const { return mFilter; }
      private:
        KNArticleFilter *mFilter;
    };

    /** Returns the index of the list view item associated with the given filter.
     * @param l The list widget to search in.
     * @param f The filter to search.
     */
    int findItem( QListWidget *l, KNArticleFilter *f );

    QListWidget *mFilterList;
    QListWidget *mMenuList;

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
    void slotSelectionChangedFilter();
    void slotSelectionChangedMenu();

};


/** Configuration widget for technical posting settings. */
class KNODE_EXPORT PostNewsTechnicalWidget : public KCModule, KNode::Ui::PostNewsTechnicalWidgetBase
{
  Q_OBJECT
  public:
    /** Create a new configuration widget for technical posting settings. */
    PostNewsTechnicalWidget( const KComponentData &inst, QWidget *parent = 0 );

    /** Reimplemented from KCModule. */
    virtual void load();
    /** Reimplemented from KCModule. */
    virtual void save();

  private slots:
    void slotSelectionChanged();
    void slotAddBtnClicked();
    void slotDelBtnClicked();
    void slotEditBtnClicked();

};


/** Dialog to edit additional headers. */
class KNODE_EXPORT XHeaderConfDialog : public KDialog
{
  public:
    /** Create a new dialog to edit an additional header.
     * @param h The header to edit.
     * @param parent The parent widget.
     */
    XHeaderConfDialog( const QString &h = QString(), QWidget *parent = 0 );
    /** Destructor. */
    ~XHeaderConfDialog();

    /** Returns the entered/modified header. */
    QString result() const;

  private:
    KLineEdit *mNameEdit, *mValueEdit;
};


/** Composer configuration widget. */
class KNODE_EXPORT PostNewsComposerWidget : public KCModule
{
  public:
    /** Create a new composer configuration widget.
     * @param parent The parent widget.
     */
    PostNewsComposerWidget( const KComponentData &inst, QWidget *parent = 0 );
};


/** Spell-checking configuration widget. */
class KNODE_EXPORT PostNewsSpellingWidget : public KCModule {

  public:
    PostNewsSpellingWidget( const KComponentData &inst, QWidget *parent = 0 );
    ~PostNewsSpellingWidget();

    void save();

  protected:
  Sonnet::ConfigWidget *c_conf;

};


/** Privacy configuration widget. */
class KNODE_EXPORT PrivacyWidget : public KCModule {

  Q_OBJECT

  public:
    PrivacyWidget( const KComponentData &inst, QWidget *parent = 0 );
    ~PrivacyWidget();

    void save();

  protected:
    Kpgp::Config *c_onf;
};



//BEGIN: Cleanup configuration -----------------------------------------------

/** Configuration widget for group expireration */
class KNODE_EXPORT GroupCleanupWidget : public QWidget {

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
    QGroupBox *mExpGroup;
    Cleanup *mData;

  private slots:
    void slotDefaultToggled( bool state );
    void expDaysChanged( int value );
    void expReadDaysChanged( int value );
    void expUnreadDaysChanged( int value );
};


/** Global cleanup configuration widget */
class KNODE_EXPORT CleanupWidget : public KCModule {

  Q_OBJECT

  public:
    CleanupWidget( const KComponentData &inst, QWidget *parent = 0 );
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

} //KNode

#endif //KNCONFIGWIDGETS_H
