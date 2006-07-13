/*
 * Demo app which shows off the ConversationView
 *
 * copyright (c) Aron Bostrom <Aron.Bostrom at gmail.com>, 2006 
 *
 * this library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <QApplication>
#include <QSplitter>
#include <QListView>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QObject>
#include <QList>
#include <QLabel>
#include <QScrollArea>
#include <QAbstractItemDelegate>
#include <QStringList>
#include <QWidget>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QFontMetrics>
#include <QMainWindow>
#include <QStatusBar>
#include <QMenu>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kstatusbar.h>
#include <ksqueezedtextlabel.h>
#include <kmenubar.h>
#include <kmenu.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kstdaction.h>
//#include <ktreewidgetsearchline.h>

#include "foldermodel.h"
#include "conversationdelegate.h"
#include "conversationview.h"
#include "conversationwidget.h"
#include "mailview.h"
#include "folderproxymodel.h"
//#include "conversationlistview.h"

static const char description[] =
  I18N_NOOP("A preview of the Google Summer of Code project GMail-style Conversation View for KMail.");

static const char version[] = "0.1";

static KCmdLineOptions options[] =
{
//    { "+[URL]", I18N_NOOP( "Document to open" ), 0 },
  KCmdLineLastOption
};

int main(int argc, char **argv)
{
  KAboutData about("conversationview", I18N_NOOP("conversationview"), version, description,
		     KAboutData::License_LGPL, "(C) 2006 Aron Boström", 0, 0, "Aron.Bostrom@gmail.com");
  about.addAuthor( "Aron Boström", 0, "Aron.Bostrom@gmail.com" );
  KCmdLineArgs::init(argc, argv, &about);
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication app;

  // no session.. just start up normally
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  QSplitter *splitter = new QSplitter;

  QStringList *me = new QStringList;
  *me << "Aron Bostrom" << "Hrafnahnef" << "Syllten";
  FolderModel *model = new FolderModel(me);

  FolderProxyModel *proxyModel = new FolderProxyModel(model);
  proxyModel->sort(1, Qt::DescendingOrder);
  MailView *mail = new MailView(proxyModel);
  ConversationView *cView = new ConversationView(proxyModel);
  ConversationWidget *cWidget = new ConversationWidget(cView);
  DummyKonadiAdapter *data = new DummyKonadiAdapter(model);



//  QItemSelectionModel *selection = new QItemSelectionModel(proxyModel);
//  cView->setSelectionModel(selection);

  QObject::connect(cView, SIGNAL(clicked(const QModelIndex&)), mail, SLOT(setConversation(const QModelIndex)));
  QObject::connect(cView, SIGNAL(activated(const QModelIndex&)), mail, SLOT(setConversation(const QModelIndex)));
  QObject::connect(splitter, SIGNAL(splitterMoved(int, int)), cView, SLOT(updateWidth(int, int)));

  splitter->addWidget(cWidget);
  splitter->addWidget(mail);
  splitter->setStretchFactor(0, 0);
  splitter->setStretchFactor(1, 1);
  QList<int> sizes;
  sizes << 510 << 300;
  splitter->setSizes(sizes);

  KMainWindow *mainWindow = new KMainWindow;
  mainWindow->setCentralWidget(splitter);
  mainWindow->setWindowTitle(i18n("Conversations for KMail"));
//  mainWindow->setMinimumWidth(900);
  KSqueezedTextLabel *statusLabel = new KSqueezedTextLabel("No Akonadi backend loaded. Using dummy data.");
  mainWindow->statusBar()->addWidget(statusLabel, 1);

  KStdAction::copy(mainWindow, SLOT(slotCopy()), mainWindow->actionCollection())->setWhatsThis(i18n("Copy\n\nCopies the selected text to the clipboard."));
  KStdAction::print(mail, SLOT(slotPrint()), mainWindow->actionCollection());
  KStdAction::quit(mail, SLOT(slotQuit()), mainWindow->actionCollection());
  mainWindow->createGUI();
  app.setMainWidget(mainWindow);
  mainWindow->show();
  args->clear();
  return app.exec();
}
