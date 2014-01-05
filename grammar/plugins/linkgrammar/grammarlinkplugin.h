/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "grammar/grammarplugin_p.h"
extern "C" {
#include <link-grammar/link-includes.h>
}
#ifndef GRAMMARLINKPLUGIN_H
#define GRAMMARLINKPLUGIN_H

class GrammarLinkPlugin : public Grammar::GrammarPlugin
{
public:
    GrammarLinkPlugin(const QString &language);
    ~GrammarLinkPlugin();
private:
    Dictionary mDict;
    Parse_Options mOpts;
};

#endif // GRAMMARLINKPLUGIN_H
