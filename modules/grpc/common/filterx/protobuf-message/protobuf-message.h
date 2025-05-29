/*
 * Copyright (c) 2025 Axoflow
 * Copyright (c) 2025 shifter
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
 *
 */

#ifndef OBJECT_PROTOBUFMESSAGE_H
#define OBJECT_PROTOBUFMESSAGE_H

#include "syslog-ng.h"

#include "compat/cpp-start.h"
#include "filterx/filterx-object.h"
#include "filterx/expr-function.h"

FILTERX_FUNCTION_DECLARE(protobuf_message);
FilterXExpr *filterx_protobuf_message_new_from_args(FilterXFunctionArgs *args, GError **error);

#define FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA_FILE "schema_file"
#define FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA "schema"

#define FILTERX_FUNC_PROTOBUF_MESSAGE_USAGE "Usage: protobuf_message({dict}, [" \
FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA"={dict}(not yet implemented)," \
FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA_FILE"={string literal}])"

#include "compat/cpp-end.h"

#endif
