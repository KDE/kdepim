#include <QApplication>
//#include <QSplitter>
#include <QListView>
#include <QItemSelectionModel>
#include <QObject>
#include <QList>
#include <QScrollArea>
#include <QAbstractItemDelegate>

#include "foldermodel.h"
#include "conversationdelegate.h"
// #include "mydisplaywidget.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
//	QSplitter *splitter = new QSplitter;

  DummyKonadiAdapter data;

  QAbstractItemModel *model = new FolderModel(data);

  QListView *conversationList = new QListView;
  conversationList->setModel(model);

  conversationList->horizontalHeader()->hide();
  conversationList->verticalHeader()->hide();
  ConversationDelegate *delegate = new ConversationDelegate(this)
  conversationList->setItemDelegate(delegate);

//   ConversationDisplay *conversationDisplay = new ConversationDisplay(this, &data);
//   QScrollArea *scrollArea = new QScrollArea;
//   scrollArea->setWidget(conversationDisplay);
//   scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  QItemSelectionModel *selection = new QItemSelectionModel(model);
  conversationList->setSelectionModel(selection);

//   QObject::connect(conversationList, SIGNAL(clicked(const QModelIndex&)),
//                    conversationDisplay, SLOT(setConversation(const QModelIndex)));
//   QObject::connect(conversationList, SIGNAL(activated(const QModelIndex&)),
//                    conversationDisplay, SLOT(setConversation(const QModelIndex)));

/*  splitter->setWindowTitle("Conversations for KMail");
  splitter->addWidget(conversationList);
  splitter->addWidget(conversationDisplay);
  splitter->setStretchFactor(0, 0);
  splitter->setStretchFactor(1, 1);
  QList<int> sizes;
  sizes << 275 << 650;
  splitter->setSizes(sizes);

  splitter->show();*/
  conversationList->show();
  return app.exec();
}


