
#include "mobile_mainview.h"

#include <QBoxLayout>
#include <QDeclarativeView>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>

#include <KStandardDirs>

#include <Akonadi/ChangeRecorder>
#include <Akonadi/ItemFetchScope>

#include "tripmodel.h"
#include <qdeclarative.h>
#include <mobile/lib/calendar/kcalitembrowseritem.h>
#include <itemselection.h>
#include <messageviewitem.h>

using namespace Akonadi;

MobileMainview::MobileMainview(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  resize(800, 480);
  QHBoxLayout *layout = new QHBoxLayout(this);

  ChangeRecorder *tripRec = new ChangeRecorder(this);
  tripRec->itemFetchScope().fetchFullPayload(true);
  TripModel *tripModel = new TripModel(tripRec, this);

  qmlRegisterType<CalendarSupport::KCal::KCalItemBrowserItem>( "org.kde.kcal", 4, 5, "IncidenceView" );
  qmlRegisterType<MessageViewer::MessageViewItem>( "org.kde.messageviewer", 4, 5, "MessageView" );

  QDeclarativeView *view = new QDeclarativeView;
  view->setResizeMode(QDeclarativeView::SizeRootObjectToView);

  QDeclarativeContext *context = view->engine()->rootContext();

  context->setContextProperty("_tripModel", tripModel);

  view->setSource(QUrl(KStandardDirs::locate( "appdata", "main.qml" )));

  layout->addWidget(view);
}
