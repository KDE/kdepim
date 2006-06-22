#include <QApplication>
//#include <QSplitter>
#include <QListView>
#include <QItemSelectionModel>
#include <QObject>
#include <QList>
#include <QScrollArea>

#include "foldermodel.h"
// #include "mydisplaywidget.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
//   QSplitter *splitter = new QSplitter;

  DummyKonadiAdapter data;

  QAbstractItemModel *model = new FolderModel(data);

  QListView *list = new QListView;
  list->setModel(model);

//   MyDisplayWidget *conversationDisplay = new MyDisplayWidget(this, data);
//   QScrollArea *scrollArea = new QScrollArea;
//   scrollArea->setWidget(conversationDisplay);
//   scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  QItemSelectionModel *selection = new QItemSelectionModel(model);
  list->setSelectionModel(selection);

//   QObject::connect(list, SIGNAL(clicked(const QModelIndex&)),
//                    conversationDisplay, SLOT(setConversation(const QModelIndex)));
//   QObject::connect(list, SIGNAL(activated(const QModelIndex&)),
//                    conversationDisplay, SLOT(setConversation(const QModelIndex)));

/*  splitter->setWindowTitle("Conversations for KMail");
  splitter->addWidget(list);
  splitter->addWidget(conversationDisplay);
  splitter->setStretchFactor(0, 0);
  splitter->setStretchFactor(1, 1);*/
/*  QList<int> sizes;
  sizes << 275 << 650;
  splitter->setSizes(sizes);
  splitter->show();*/
  list->show();
  return app.exec();
}


