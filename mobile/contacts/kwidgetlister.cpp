
#include "kwidgetlister.h"

#include <QBoxLayout>

class KWidgetLister::Private
{
  public:
    Private( KWidgetLister *qq )
      : q( qq ), mOrientation( Qt::Vertical )
    {
      mLayout = new QBoxLayout( QBoxLayout::TopToBottom, q );
      mLayout->setMargin( 0 );
      mLayout->setAlignment( Qt::AlignTop );
    }

    void addWidget( QWidget *widget )
    {
      if ( !widget )
        return;

      q->connect( widget, SIGNAL(remove(QWidget*)), q, SLOT(removeWidget(QWidget*)) );
      mWidgets.append( widget );
      mLayout->addWidget( widget );
    }

    void removeWidget( QWidget *widget )
    {
      if ( !mWidgets.contains( widget ) ) {
        qDebug( "Warning: try to remove non-existing widget" );
        return;
      }

      emit q->aboutToBeRemoved( widget );

      mLayout->removeWidget( widget );
      mWidgets.removeAll( widget );
      widget->hide();
      widget->deleteLater();
    }

    KWidgetLister *q;
    Qt::Orientation mOrientation;
    QList<QWidget*> mWidgets;
    QBoxLayout *mLayout;
};

KWidgetLister::KWidgetLister( QWidget *parent )
  : QWidget( parent ), d( new Private( this ) )
{
}

KWidgetLister::~KWidgetLister()
{
  delete d;
}

void KWidgetLister::setOrientation( Qt::Orientation orientation )
{
  d->mOrientation = orientation;
  if ( d->mOrientation == Qt::Horizontal )
    d->mLayout->setDirection( QBoxLayout::LeftToRight );
  else
    d->mLayout->setDirection( QBoxLayout::TopToBottom );
}

Qt::Orientation KWidgetLister::orientation() const
{
  return d->mOrientation;
}

void KWidgetLister::addWidget( QWidget *widget )
{
  d->addWidget( widget );
}

void KWidgetLister::clear()
{
  foreach ( QWidget *widget, d->mWidgets )
    d->removeWidget( widget );
}

int KWidgetLister::count() const
{
  return d->mWidgets.count();
}

QWidget* KWidgetLister::widget( int index ) const
{
  if ( index < 0 || index >= d->mWidgets.count() )
    return 0;

  return d->mWidgets.at( index );
}

#include "moc_kwidgetlister.cpp"
