
#ifndef KSYNC_KONNECTOR_CONFIG_WIDGET_H
#define KSYNC_KONNECTOR_CONFIG_WIDGET_H

#include <qwidget.h>

#include <kapabilities.h>

namespace KSync {
    /**
     * This is a baseclass for a configuration GUI provided
     * by a Konnector
     * It provides methods to get a Kapaibility and to set
     * Kapabilities
     *
     */
    class ConfigWidget : public QWidget {
        Q_OBJECT
    public:
        /** normal QWidget like c'tor + a Kapabilities object */
        ConfigWidget( const Kapabilities&, QWidget* parent,  const char* name );

        /** another c'tor */
        ConfigWidget( QWidget* parent, const char* name );

        /** d'tor */
        virtual ~ConfigWidget();

        /** returns a capability of the widget */
        virtual Kapabilities capabilities() const = 0;

        /** set this widget to show a capability */
        virtual void setCapabilities( const Kapabilities& ) = 0;
    private:
        class ConfigWidgetPrivate;
        ConfigWidgetPrivate* d;

    };
};
#endif
