// Author: Mark Kretschmann (C) Copyright 2004
// Copyright: See COPYING file that comes with this distribution

#ifndef AKREGATOR_PLUGIN_H
#define AKREGATOR_PLUGIN_H

#include "akregator_export.h"

#define AKREGATOR_EXPORT_PLUGIN( classname ) \
    extern "C" { \
         Akregator::Plugin* create_plugin() { return new classname; } \
    }

#include <tqmap.h>
#include <tqstring.h>


namespace Akregator
{
//    class PluginConfig;

    class AKREGATOR_EXPORT Plugin
    {
        public:
            virtual ~Plugin();

            virtual bool init() = 0;
            /**
             * TODO @param parent you must parent the widget to parent
             * @return the configure widget for your plugin, create it on the heap!
             */
             //TODO rename configureWidget( TQWidget *parent )
            // virtual PluginConfig* configure() const { return 0; }

            void addPluginProperty( const TQString& key, const TQString& value );
            TQString pluginProperty( const TQString& key );
            bool hasPluginProperty( const TQString& key );

        protected:
            Plugin();

        private:
            TQMap<TQString, TQString> m_properties;
    };

} //namespace Akregator


#endif /* AKREGATOR_PLUGIN_H */


