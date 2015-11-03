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

#ifndef __MESSAGELIST_CORE_MODELINVARIANTINDEX_P_H__
#define __MESSAGELIST_CORE_MODELINVARIANTINDEX_P_H__

#include "core/modelinvariantindex.h"

namespace MessageList
{

namespace Core
{

class ModelInvariantIndex::Private
{
public:
    int mModelIndexRow;                   ///< The row that this index referenced at the time it was emitted
    uint mRowMapperSerial;                ///< The serial that was current in the RowMapper at the time the invariant index was emitted
    ModelInvariantRowMapper *mRowMapper;  ///< The mapper that this invariant index is attached to

    int modelIndexRow() const
    {
        return mModelIndexRow;
    };
    uint rowMapperSerial() const
    {
        return mRowMapperSerial;
    };
    void setModelIndexRowAndRowMapperSerial(int modelIndexRow, uint rowMapperSerial)
    {
        mModelIndexRow = modelIndexRow;
        mRowMapperSerial = rowMapperSerial;
    };
    ModelInvariantRowMapper *rowMapper() const
    {
        return mRowMapper;
    };
    void setRowMapper(ModelInvariantRowMapper *mapper)
    {
        mRowMapper = mapper;
    };
};

} // namespace Core

} // namespace MessageList

#endif //!__MESSAGELIST_CORE_MODELINVARIANTINDEX_P_H__
