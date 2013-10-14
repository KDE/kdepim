/*
    Copyright (c) 2003 Andreas Gungl <a.gungl@gmx.de>
    Copyright (c) 2012 Laurent Montel <montel@kde.org>

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

#include "filterlogdialog.h"
#include <mailcommon/filter/filterlog.h>
#include <messageviewer/utils/autoqpointer.h>
#include "pimcommon/texteditor/plaintexteditor/plaintexteditorwidget.h"
#include "pimcommon/texteditor/plaintexteditor/plaintexteditor.h"

#include <kdebug.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kvbox.h>

#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QStringList>
#include <QGroupBox>
#include <QVBoxLayout>

#include <errno.h>

using namespace MailCommon;

FilterLogDialog::FilterLogDialog( QWidget * parent )
  : KDialog( parent ), mIsInitialized( false )
{
  KGlobal::locale()->insertCatalog(QLatin1String("akonadi_mailfilter_agent"));
  KGlobal::locale()->insertCatalog(QLatin1String("libpimcommon"));
  setCaption( i18n( "Filter Log Viewer" ) );
  setButtons( User1|User2|Close );
  setWindowIcon( KIcon( QLatin1String("view-filter") ) );
  setModal( false );
  setDefaultButton( Close );
  setButtonGuiItem( User1, KStandardGuiItem::clear() );
  setButtonGuiItem( User2, KStandardGuiItem::saveAs() );
  QFrame *page = new KVBox( this );
  setMainWidget( page );

  mTextEdit = new PimCommon::PlainTextEditorWidget( page );
  mTextEdit->setReadOnly( true );
  mTextEdit->editor()->setWordWrapMode(QTextOption::NoWrap);
  const QStringList logEntries = FilterLog::instance()->logEntries();
  QStringList::ConstIterator end( logEntries.constEnd() ); 
  for ( QStringList::ConstIterator it = logEntries.constBegin();
        it != end; ++it ) {
    mTextEdit->editor()->appendHtml(*it);
  }

  mLogActiveBox = new QCheckBox( i18n("&Log filter activities"), page );
  mLogActiveBox->setChecked( FilterLog::instance()->isLogging() );
  connect( mLogActiveBox, SIGNAL(clicked()),
           this, SLOT(slotSwitchLogState()) );
  mLogActiveBox->setWhatsThis(
      i18n( "You can turn logging of filter activities on and off here. "
            "Of course, log data is collected and shown only when logging "
            "is turned on. " ) );

  mLogDetailsBox = new QGroupBox(i18n( "Logging Details" ), page );
  QVBoxLayout *layout = new QVBoxLayout;
  mLogDetailsBox->setLayout( layout );
  mLogDetailsBox->setEnabled( mLogActiveBox->isChecked() );
  connect( mLogActiveBox, SIGNAL(toggled(bool)),
           mLogDetailsBox, SLOT(setEnabled(bool)) );

  mLogPatternDescBox = new QCheckBox( i18n("Log pattern description") );
  layout->addWidget( mLogPatternDescBox );
  mLogPatternDescBox->setChecked(
      FilterLog::instance()->isContentTypeEnabled( FilterLog::PatternDescription ) );
  connect( mLogPatternDescBox, SIGNAL(clicked()),
           this, SLOT(slotChangeLogDetail()) );
  // TODO
  //QWhatsThis::add( mLogPatternDescBox,
  //    i18n( "" ) );

  mLogRuleEvaluationBox = new QCheckBox( i18n("Log filter &rule evaluation") );
  layout->addWidget( mLogRuleEvaluationBox );
  mLogRuleEvaluationBox->setChecked(
      FilterLog::instance()->isContentTypeEnabled( FilterLog::RuleResult ) );
  connect( mLogRuleEvaluationBox, SIGNAL(clicked()),
           this, SLOT(slotChangeLogDetail()) );
  mLogRuleEvaluationBox->setWhatsThis(
      i18n( "You can control the feedback in the log concerning the "
            "evaluation of the filter rules of applied filters: "
            "having this option checked will give detailed feedback "
            "for each single filter rule; alternatively, only "
            "feedback about the result of the evaluation of all rules "
            "of a single filter will be given." ) );

  mLogPatternResultBox = new QCheckBox( i18n("Log filter pattern evaluation") );
  layout->addWidget( mLogPatternResultBox );
  mLogPatternResultBox->setChecked(
      FilterLog::instance()->isContentTypeEnabled( FilterLog::PatternResult ) );
  connect( mLogPatternResultBox, SIGNAL(clicked()),
           this, SLOT(slotChangeLogDetail()) );
  // TODO
  //QWhatsThis::add( mLogPatternResultBox,
  //    i18n( "" ) );

  mLogFilterActionBox = new QCheckBox( i18n("Log filter actions") );
  layout->addWidget( mLogFilterActionBox );
  mLogFilterActionBox->setChecked(
      FilterLog::instance()->isContentTypeEnabled( FilterLog::AppliedAction ) );
  connect( mLogFilterActionBox, SIGNAL(clicked()),
           this, SLOT(slotChangeLogDetail()) );
  // TODO
  //QWhatsThis::add( mLogFilterActionBox,
  //    i18n( "" ) );

  KHBox * hbox = new KHBox( page );
  new QLabel( i18n("Log size limit:"), hbox );
  mLogMemLimitSpin = new QSpinBox( hbox );
  mLogMemLimitSpin->setMinimum( 1 );
  mLogMemLimitSpin->setMaximum( 1024 * 256 ); // 256 MB
  // value in the QSpinBox is in KB while it's in Byte in the FilterLog
  mLogMemLimitSpin->setValue( FilterLog::instance()->maxLogSize() / 1024 );
  mLogMemLimitSpin->setSuffix( i18n(" KB") );
  mLogMemLimitSpin->setSpecialValueText(
    i18nc("@label:spinbox Set the size of the logfile to unlimited.", "unlimited") );
  connect( mLogMemLimitSpin, SIGNAL(valueChanged(int)),
           this, SLOT(slotChangeLogMemLimit(int)) );
  mLogMemLimitSpin->setWhatsThis(
      i18n( "Collecting log data uses memory to temporarily store the "
            "log data; here you can limit the maximum amount of memory "
            "to be used: if the size of the collected log data exceeds "
            "this limit then the oldest data will be discarded until "
            "the limit is no longer exceeded. " ) );

  connect(FilterLog::instance(), SIGNAL(logEntryAdded(QString)),
          this, SLOT(slotLogEntryAdded(QString)));
  connect(FilterLog::instance(), SIGNAL(logShrinked()),
          this, SLOT(slotLogShrinked()));
  connect(FilterLog::instance(), SIGNAL(logStateChanged()),
          this, SLOT(slotLogStateChanged()));

  setInitialSize( QSize( 500, 500 ) );
  connect( this, SIGNAL(user1Clicked()), SLOT(slotUser1()) );
  connect( this, SIGNAL(user2Clicked()), SLOT(slotUser2()) );
  readConfig();
  mIsInitialized = true;
}

void FilterLogDialog::readConfig()
{
  KSharedConfig::Ptr config = KGlobal::config();
  KConfigGroup group( config, "FilterLog" );
  const bool isEnabled = group.readEntry( "Enabled", false );
  const bool isLogPatternDescription = group.readEntry( "LogPatternDescription", false );
  const bool isLogRuleResult = group.readEntry( "LogRuleResult", false );
  const bool isLogPatternResult = group.readEntry( "LogPatternResult", false );
  const bool isLogAppliedAction = group.readEntry( "LogAppliedAction", false );
  const int maxLogSize = group.readEntry( "maxLogSize", -1 );

  if ( isEnabled !=FilterLog::instance()->isLogging() )  
    FilterLog::instance()->setLogging( isEnabled );
  if ( isLogPatternDescription != FilterLog::instance()->isContentTypeEnabled( FilterLog::PatternDescription ) )
    FilterLog::instance()->setContentTypeEnabled( FilterLog::PatternDescription, isLogPatternDescription );
  if (  isLogRuleResult!= FilterLog::instance()->isContentTypeEnabled( FilterLog::RuleResult ) )
    FilterLog::instance()->setContentTypeEnabled( FilterLog::RuleResult, isLogRuleResult);
  if ( isLogPatternResult != FilterLog::instance()->isContentTypeEnabled( FilterLog::PatternResult ) )
    FilterLog::instance()->setContentTypeEnabled( FilterLog::PatternResult,isLogPatternResult );
  if ( isLogAppliedAction != FilterLog::instance()->isContentTypeEnabled( FilterLog::AppliedAction ) )
    FilterLog::instance()->setContentTypeEnabled( FilterLog::AppliedAction,isLogAppliedAction ); 
  if ( FilterLog::instance()->maxLogSize() != maxLogSize )
    FilterLog::instance()->setMaxLogSize( maxLogSize );  

  KConfigGroup geometryGroup( config, "Geometry" );
  const QSize size = geometryGroup.readEntry( "filterLogSize", QSize() );
  if ( size != QSize() ) {
    resize( size );
  } else {
    adjustSize();
  }
}

FilterLogDialog::~FilterLogDialog()
{
  KConfigGroup myGroup( KGlobal::config(), "Geometry" );
  myGroup.writeEntry( "filterLogSize", size() );
  myGroup.sync();
}

void FilterLogDialog::writeConfig()
{
  if ( !mIsInitialized )
    return;
  
  KSharedConfig::Ptr config = KGlobal::config();
  KConfigGroup group( config, "FilterLog" );
  group.writeEntry( "Enabled", FilterLog::instance()->isLogging() );
  group.writeEntry( "LogPatternDescription", FilterLog::instance()->isContentTypeEnabled( FilterLog::PatternDescription ) );
  group.writeEntry( "LogRuleResult", FilterLog::instance()->isContentTypeEnabled( FilterLog::RuleResult ) );
  group.writeEntry( "LogPatternResult", FilterLog::instance()->isContentTypeEnabled( FilterLog::PatternResult ) );
  group.writeEntry( "LogAppliedAction", FilterLog::instance()->isContentTypeEnabled( FilterLog::AppliedAction ) );
  group.writeEntry( "maxLogSize", ( int )( FilterLog::instance()->maxLogSize() ) );
  group.sync();
}

void FilterLogDialog::slotLogEntryAdded(const QString& logEntry )
{
  mTextEdit->editor()->appendHtml( logEntry );
}


void FilterLogDialog::slotLogShrinked()
{
  // limit the size of the shown log lines as soon as
  // the log has reached it's memory limit
  if ( mTextEdit->editor()->document()->maximumBlockCount () <= 0 ) {
    mTextEdit->editor()->document()->setMaximumBlockCount( mTextEdit->editor()->document()->blockCount() );
  }
}


void FilterLogDialog::slotLogStateChanged()
{
  mLogActiveBox->setChecked( FilterLog::instance()->isLogging() );
  mLogPatternDescBox->setChecked(
      FilterLog::instance()->isContentTypeEnabled( FilterLog::PatternDescription ) );
  mLogRuleEvaluationBox->setChecked(
      FilterLog::instance()->isContentTypeEnabled( FilterLog::RuleResult ) );
  mLogPatternResultBox->setChecked(
      FilterLog::instance()->isContentTypeEnabled( FilterLog::PatternResult ) );
  mLogFilterActionBox->setChecked(
      FilterLog::instance()->isContentTypeEnabled( FilterLog::AppliedAction ) );

  // value in the QSpinBox is in KB while it's in Byte in the FilterLog
  int newLogSize = FilterLog::instance()->maxLogSize() / 1024;
  if ( mLogMemLimitSpin->value() != newLogSize ) {
    if(newLogSize <= 0)
      mLogMemLimitSpin->setValue( 1 );
    else
      mLogMemLimitSpin->setValue( newLogSize );
  }
  writeConfig();
}


void FilterLogDialog::slotChangeLogDetail()
{
  if ( mLogPatternDescBox->isChecked() !=
       FilterLog::instance()->isContentTypeEnabled( FilterLog::PatternDescription ) )
    FilterLog::instance()->setContentTypeEnabled( FilterLog::PatternDescription,
                                                  mLogPatternDescBox->isChecked() );

  if ( mLogRuleEvaluationBox->isChecked() !=
       FilterLog::instance()->isContentTypeEnabled( FilterLog::RuleResult ) )
    FilterLog::instance()->setContentTypeEnabled( FilterLog::RuleResult,
                                                  mLogRuleEvaluationBox->isChecked() );

  if ( mLogPatternResultBox->isChecked() !=
       FilterLog::instance()->isContentTypeEnabled( FilterLog::PatternResult ) )
    FilterLog::instance()->setContentTypeEnabled( FilterLog::PatternResult,
                                                  mLogPatternResultBox->isChecked() );

  if ( mLogFilterActionBox->isChecked() !=
       FilterLog::instance()->isContentTypeEnabled( FilterLog::AppliedAction ) )
    FilterLog::instance()->setContentTypeEnabled( FilterLog::AppliedAction,
                                                  mLogFilterActionBox->isChecked() );
}


void FilterLogDialog::slotSwitchLogState()
{
  FilterLog::instance()->setLogging( mLogActiveBox->isChecked() );
}


void FilterLogDialog::slotChangeLogMemLimit( int value )
{
  mTextEdit->editor()->document()->setMaximumBlockCount( 0 ); //Reset value
  if(value == 1) //unilimited
    FilterLog::instance()->setMaxLogSize(-1);
  else
    FilterLog::instance()->setMaxLogSize( value * 1024 );
}


void FilterLogDialog::slotUser1()
{
  FilterLog::instance()->clear();
  mTextEdit->editor()->clear();
}


void FilterLogDialog::slotUser2()
{
  KUrl url;
  MessageViewer::AutoQPointer<KFileDialog> fdlg( new KFileDialog( url, QString(), this) );

  fdlg->setMode( KFile::File );
  fdlg->setSelection( QLatin1String("kmail-filter.html") );
  fdlg->setOperationMode( KFileDialog::Saving );
  fdlg->setConfirmOverwrite(true);
  if ( fdlg->exec() == QDialog::Accepted && fdlg )
  {
    const QString fileName = fdlg->selectedFile();

    if ( !fileName.isEmpty() && !FilterLog::instance()->saveToFile( fileName ) )
    {
      KMessageBox::error( this,
                          i18n( "Could not write the file %1:\n"
                                "\"%2\" is the detailed error description.",
                                fileName,
                                QString::fromLocal8Bit( strerror( errno ) ) ),
                          i18n( "KMail Error" ) );
    }
  }
}

#include "filterlogdialog.moc"
