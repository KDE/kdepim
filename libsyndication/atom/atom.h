/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef LIBSYNDICATION_ATOM_ATOM_H
#define LIBSYNDICATION_ATOM_ATOM_H

#include <libsyndication/atom/category.h>
#include <libsyndication/atom/constants.h>
#include <libsyndication/atom/content.h>
#include <libsyndication/atom/document.h>
#include <libsyndication/atom/entry.h>
#include <libsyndication/atom/generator.h>
#include <libsyndication/atom/link.h>
#include <libsyndication/atom/parser.h>
#include <libsyndication/atom/person.h>
#include <libsyndication/atom/source.h>

namespace Syndication {

/** 
 * Atom parser and model classes, representing
 * Atom 1.0 documents (Atom 0.3 documents are
 * converted by the parser) 
 */
namespace Atom {}

}

#endif // LIBSYNDICATION_ATOM_ATOM_H
