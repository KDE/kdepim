
#include "configwidget.h"

using namespace KSync;

ConfigWidget::ConfigWidget( const Kapabilities&, QWidget *parent,  const char* name )
    : QWidget( parent, name ) {}
ConfigWidget::ConfigWidget( QWidget* parent, const char* name )
    : QWidget( parent, name ) {}
ConfigWidget::~ConfigWidget() {}



#include "configwidget.moc"
