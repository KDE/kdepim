/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#include "core/modelinvariantindex.h"
#include "core/modelinvariantindex_p.h"
#include "core/modelinvariantrowmapper.h"
#include "core/modelinvariantrowmapper_p.h"

using namespace MessageList::Core;

ModelInvariantIndex::ModelInvariantIndex()
    : d(new Private)
{
    d->mRowMapper = 0;
}

ModelInvariantIndex::~ModelInvariantIndex()
{
    if (d->mRowMapper) {
        d->mRowMapper->d->indexDead(this);
    }

    delete d;
}

bool ModelInvariantIndex::isValid() const
{
    return d->mRowMapper != 0;
}

int ModelInvariantIndex::currentModelIndexRow()
{
    if (d->mRowMapper) {
        return d->mRowMapper->modelInvariantIndexToModelIndexRow(this);
    }
    return -1;
}
