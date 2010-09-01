/*  -*- mode: C++ -*-
    This file is part of KMail.
    Copyright (c) 2003 Andreas Gungl <a.gungl@gmx.de>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/
#ifndef KMAIL_ANTISPAMWIZARD_H
#define KMAIL_ANTISPAMWIZARD_H

#include "simplefoldertree.h"

#include <kconfig.h>
#include <klistbox.h>
#include <kwizard.h>

#include <tqcheckbox.h>
#include <tqdict.h>
#include <tqlayout.h>

class KActionCollection;
class KMFolder;
class KMFolderTree;
class TQLabel;

namespace KMail {

  class FolderRequester;

  class ASWizInfoPage;
  class ASWizSpamRulesPage;
  class ASWizVirusRulesPage;
  class ASWizSummaryPage;

  //---------------------------------------------------------------------------
  /**
    @short KMail anti-spam wizard.
    @author Andreas Gungl <a.gungl@gmx.de>

    The wizard helps to create filter rules to let KMail operate
    with external anti-spam tools. The wizard tries to detect the
    tools, but the user can overide the preselections.
    Then the user can decide what funtionality shall be supported
    by the created filter rules.
    The wizard will append the created filter rules after the
    last existing rule to keep possible conflicts with existing
    filter configurations minimal.

    Anti-virus support was added by Fred Emmott <fred87@users.sf.net>

    The configuration for the tools to get checked and set up
    is read from a config file. The structure of the file is as
    following:
    <pre>
    [General]
    tools=1

    [Spamtool #1]
    Ident=spamassassin
    Version=0
    Priority=1
    VisibleName=&Spamassassin
    Executable=spamassassin -V
    URL=http://spamassassin.org
    PipeFilterName=SpamAssassin Check
    PipeCmdDetect=spamassassin -L
    ExecCmdSpam=sa-learn --spam --no-rebuild --single
    ExecCmdHam=sa-learn --ham --no-rebuild --single
    DetectionHeader=X-Spam-Flag
    DetectionPattern=yes
    DetectionPattern2=
    DetectionOnly=0
    UseRegExp=0
    SupportsBayes=1
    SupportsUnsure=0
    ServerSided=0
    type=spam
    </pre>
    The name of the config file is kmail.antispamrc
    and it's expected in the config dir of KDE.

  */
  class AntiSpamWizard : public KWizard
  {
    Q_OBJECT

    public:
      /** The wizard can be used for setting up anti-spam tools and for
          setting up anti-virus tools.
      */
      enum WizardMode { AntiSpam, AntiVirus };

      /** Constructor that needs to initialize from the main folder tree
        of KMail.
        @param mode The mode the wizard should run in.
        @param parent The parent widget for the wizard.
        @param mainFolderTree The main folder tree from which the folders
          are copied to allow the selection of a spam folder in a tree
          within one of the wizard pages.
      */
      AntiSpamWizard( WizardMode mode,
                      TQWidget * parent, KMFolderTree * mainFolderTree );

    protected:
      /**
        Instances of this class store the settings for one tool as read from
        the config file. Visible name and What's this text can not get
        translated!
      */
      class SpamToolConfig
      {
        public:
          SpamToolConfig() {}
          SpamToolConfig( TQString toolId, int configVersion, int prio,
                        TQString name, TQString exec, TQString url, TQString filter,
                        TQString detection, TQString spam, TQString ham,
                        TQString header, TQString pattern, TQString pattern2,
                        TQString serverPattern,
                        bool detectionOnly, bool regExp, bool bayesFilter, 
                        bool tristateDetection, WizardMode type );

          int getVersion() const { return mVersion; }
          int getPrio() const { return mPrio; }
          TQString getId()  const { return mId; }
          TQString getVisibleName()  const { return mVisibleName; }
          TQString getExecutable() const { return mExecutable; }
          TQString getWhatsThisText() const { return mWhatsThisText; }
          TQString getFilterName() const { return mFilterName; }
          TQString getDetectCmd() const { return mDetectCmd; }
          TQString getSpamCmd() const { return mSpamCmd; }
          TQString getHamCmd() const { return mHamCmd; }
          TQString getDetectionHeader() const { return mDetectionHeader; }
          TQString getDetectionPattern() const { return mDetectionPattern; }
          TQString getDetectionPattern2() const { return mDetectionPattern2; }
          TQString getServerPattern() const { return mServerPattern; }
          bool isServerBased() const;
          bool isDetectionOnly() const { return mDetectionOnly; }
          bool isUseRegExp() const { return mUseRegExp; }
          bool useBayesFilter() const { return mSupportsBayesFilter; }
          bool hasTristateDetection() const { return mSupportsUnsure; }
          WizardMode getType() const { return mType; }
          // convinience methods for types
          bool isSpamTool() const { return ( mType == AntiSpam ); }
          bool isVirusTool() const { return ( mType == AntiVirus ); }

        private:
          // used to identifiy configs for the same tool
          TQString mId;
          // The version of the config data, used for merging and
          // detecting newer configs
          int mVersion;
          // the priority of the tool in the list presented to the user
          int mPrio;
          // the name as shown by the checkbox in the dialog page
          TQString mVisibleName;
          // the command to check the existance of the tool
          TQString mExecutable;
          // the What's This help text (e.g. url for the tool)
          TQString mWhatsThisText;
          // name for the created filter in the filter list
          TQString mFilterName;
          // pipe through cmd used to detect spam messages
          TQString mDetectCmd;
          // pipe through cmd to let the tool learn a spam message
          TQString mSpamCmd;
          // pipe through cmd to let the tool learn a ham message
          TQString mHamCmd;
          // by which header are messages marked as spam
          TQString mDetectionHeader;
          // what header pattern is used to mark spam messages
          TQString mDetectionPattern;
          // what header pattern is used to mark unsure messages
          TQString mDetectionPattern2;
          // what header pattern is used in the account to check for a certain server
          TQString mServerPattern;
          // filter cannot search actively but relies on pattern by regExp or contain rule
          bool mDetectionOnly;
          // filter searches for the pattern by regExp or contain rule
          bool mUseRegExp;
          // can the tool learn spam and ham, has it a bayesian algorithm
          bool mSupportsBayesFilter;
          // differentiate between ham, spam and a third "unsure" state
          bool mSupportsUnsure;
          // Is the tool AntiSpam or AntiVirus
          WizardMode mType;
      };

      /**
        Instances of this class control reading the configuration of the
        anti-spam tools from global and user config files as well as the
        merging of different config versions.
      */
      class ConfigReader
      {
        public:
          ConfigReader( WizardMode mode,
                        TQValueList<SpamToolConfig> & configList );
          ~ConfigReader( );

          TQValueList<SpamToolConfig> & getToolList() { return mToolList; }

          void readAndMergeConfig();

        private:
          TQValueList<SpamToolConfig> & mToolList;
          KConfig *mConfig;
          WizardMode mMode;

          SpamToolConfig readToolConfig( KConfigGroup & configGroup );
          SpamToolConfig createDummyConfig();

          void mergeToolConfig( SpamToolConfig config );
          void sortToolList();
      };

      /** Evaluate the settings made and create the appropriate filter rules. */
      void accept();

    protected slots:
      /** Modify the status of the wizard to reflect the selection of spam tools. */
      void checkProgramsSelections();
      /** Modify the status of the wizard to reflect the selected functionality. */
      void checkVirusRulesSelections();
      /** Check if the spam tools are available via the PATH */
      void checkToolAvailability();
      /** Show a help topic */
      void slotHelpClicked();
      /** Create the summary text based on the current settings */
      void slotBuildSummary();

    private:
      /* Check for the availability of an executible along the PATH */
      int checkForProgram( const TQString &executable );
      /* generic checks if any option in a page is checked */
      bool anyVirusOptionChecked();
      /* convenience method calling the appropriate filter manager method */
      const TQString uniqueNameFor( const TQString & name );
      /* convenience method to sort out new and existing filters */
      void sortFilterOnExistance( const TQString & intendedFilterName,
                                  TQString & newFilters, 
                                  TQString & replaceFilters );

      /* The pages in the wizard */
      ASWizInfoPage * mInfoPage;
      ASWizSpamRulesPage * mSpamRulesPage;
      ASWizVirusRulesPage * mVirusRulesPage;
      ASWizSummaryPage * mSummaryPage;

      /* The configured tools and it's settings to be used in the wizard. */
      TQValueList<SpamToolConfig> mToolList;

      /* Are any spam tools selected? */
      bool mSpamToolsUsed;
      /* Are any virus tools selected? */
      bool mVirusToolsUsed;

      WizardMode mMode;
  };


  //---------------------------------------------------------------------------
  class ASWizPage : public QWidget
  {
    public:
      ASWizPage( TQWidget *parent, const char *name, 
                 const TQString *bannerName = 0);

    protected:
      TQBoxLayout *mLayout;

    private:
      TQLabel *mBannerLabel;
  };


  //---------------------------------------------------------------------------
  class ASWizInfoPage : public ASWizPage
  {
    Q_OBJECT

    public:
      ASWizInfoPage( AntiSpamWizard::WizardMode mode,
                     TQWidget *parent, const char *name );

      void setScanProgressText( const TQString &toolName );
      void addAvailableTool( const TQString &visibleName );
      bool isProgramSelected( const TQString &visibleName );

    private slots:
      void processSelectionChange();

    signals:
      void selectionChanged();

    private:
      TQLabel *mIntroText;
      TQLabel *mScanProgressText;
      TQLabel *mSelectionHint;
      KListBox *mToolsList;
  };

  //---------------------------------------------------------------------------
  class ASWizSpamRulesPage : public ASWizPage
  {
    Q_OBJECT

    public:
      ASWizSpamRulesPage( TQWidget * parent, const char * name, KMFolderTree * mainFolderTree );

      bool markAsReadSelected() const;
      bool moveSpamSelected() const;
      bool moveUnsureSelected() const;

      TQString selectedSpamFolderName() const;
      TQString selectedUnsureFolderName() const;

      void allowUnsureFolderSelection( bool enabled );

    private slots:
      void processSelectionChange();
      void processSelectionChange( KMFolder* );

    signals:
      void selectionChanged();

    private:
      TQCheckBox * mMarkRules;
      TQCheckBox * mMoveSpamRules;
      TQCheckBox * mMoveUnsureRules;
      FolderRequester *mFolderReqForSpamFolder;
      FolderRequester *mFolderReqForUnsureFolder;
  };

  //-------------------------------------------------------------------------
  class ASWizVirusRulesPage : public ASWizPage
  {
    Q_OBJECT

    public:
      ASWizVirusRulesPage( TQWidget * parent, const char * name, KMFolderTree * mainFolderTree );

      bool pipeRulesSelected() const;
      bool moveRulesSelected() const;
      bool markReadRulesSelected() const;

      TQString selectedFolderName() const;

    private slots:
      void processSelectionChange();
    signals:
      void selectionChanged();

    private:
      TQCheckBox * mPipeRules;
      TQCheckBox * mMoveRules;
      SimpleFolderTree *mFolderTree;
      TQCheckBox * mMarkRules;
  };

  //---------------------------------------------------------------------------
  class ASWizSummaryPage : public ASWizPage
  {
    Q_OBJECT

    public:
      ASWizSummaryPage( TQWidget * parent, const char * name );

      void setSummaryText( const TQString & text );

    private:
      TQLabel * mSummaryText;
  };


} // namespace KMail

#endif // KMAIL_ANTISPAMWIZARD_H
