/*
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
#include <QScrollArea>
#include <QAbstractItemDelegate>
#include <QStringList>
#include <QSortFilterProxyModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>
#include <QFontMetrics>

#include "foldermodel.h"
#include "conversationdelegate.h"
#include "mailview.h"
//#include "conversationlistview.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QSplitter *splitter = new QSplitter;

  DummyKonadiAdapter *data = new DummyKonadiAdapter;

  FolderModel *model = new FolderModel(data);
  QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel;
  proxyModel->setSourceModel(model);
  proxyModel->sort(1, Qt::AscendingOrder);

  MailView *mail = new MailView(model);

  QListView *conversationList = new QListView;
  conversationList->setModel(proxyModel);
//  conversationList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QHeaderView *header = new QHeaderView(Qt::Horizontal);
	header->setModel(proxyModel);
	header->setStretchLastSection(true);
	header->setDefaultSectionSize(185);
	header->resizeSection(0, 185);
	header->setResizeMode(0, QHeaderView::Interactive);
		

  QStringList me;
  me << "Aron Bostrom" << "Hrafnahnef" << "Syllten";
  ConversationDelegate *delegate = new ConversationDelegate(model, proxyModel, me, header);
  conversationList->setItemDelegate(delegate);
  conversationList->setUniformItemSizes(true);

  
  QWidget *myWidget = new QWidget;
  QGridLayout *layout = new QGridLayout;
  layout->setMargin(0);
  layout->setSpacing(0);
  layout->addWidget(header);
  header->setMinimumHeight(QFontMetrics(header->font()).height()+4);
  header->setMaximumHeight(QFontMetrics(header->font()).height()+6);
  header->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
  layout->addWidget(conversationList);
  layout->setRowStretch(0, 0);
  layout->setRowStretch(1, 100);
  myWidget->setLayout(layout);

//  QItemSelectionModel *selection = new QItemSelectionModel(proxyModel);
//  conversationList->setSelectionModel(selection);

  QObject::connect(conversationList, SIGNAL(clicked(const QModelIndex&)), mail, SLOT(setConversation(const QModelIndex)));
  QObject::connect(conversationList, SIGNAL(activated(const QModelIndex&)), mail, SLOT(setConversation(const QModelIndex)));
  QObject::connect(mail, SIGNAL(textChanged()), mail, SLOT(updateHeight()));
  QObject::connect(splitter, SIGNAL(splitterMoved(int, int)), delegate, SLOT(updateWidth(int, int)));
  QObject::connect(header, SIGNAL(sectionResized(int, int, int)), conversationList, SLOT(updateAuthorsWidth(int, int, int)));
	
  splitter->setWindowTitle("Conversations for KMail");
  splitter->addWidget(myWidget);
  splitter->addWidget(mail);
  splitter->setStretchFactor(0, 0);
  splitter->setStretchFactor(1, 1);
  QList<int> sizes;
  sizes << 510 << 300;
  splitter->setSizes(sizes);

  splitter->setMinimumWidth(900);
  splitter->show();
  return app.exec();
}


