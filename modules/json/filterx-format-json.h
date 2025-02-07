/*
 * Copyright (c) 2024 Attila Szakacs
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 */
#ifndef FILTERX_FORMAT_JSON_H_INCLUDED
#define FILTERX_FORMAT_JSON_H_INCLUDED

#include "filterx/filterx-object.h"
#include "filterx/expr-function.h"

FilterXObject *filterx_format_json_call(FilterXExpr *s, FilterXObject *args[], gsize args_len);

FILTERX_SIMPLE_FUNCTION_DECLARE(format_json);

#endif
