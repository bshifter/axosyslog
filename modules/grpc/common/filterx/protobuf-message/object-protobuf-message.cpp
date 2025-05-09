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

#include "object-protobuf-message.hpp"

#include "compat/cpp-start.h"

#include "filterx/object-extractor.h"
#include "filterx/object-string.h"
#include "filterx/object-datetime.h"
#include "filterx/object-primitive.h"
#include "scratch-buffers.h"
#include "generic-number.h"

#include "compat/cpp-end.h"

#include <google/protobuf/util/json_util.h>

#include <unistd.h>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace syslogng::grpc::clickhouse::filterx;

/* C++ Implementations */
Message::Message(FilterXProtobufMessage *super_, Schema *schema) : super(super_)
{
  this->schema = schema;
}

// /* C Wrappers */

static void
_free(FilterXObject *s)
{
  FilterXProtobufMessage *self = (FilterXProtobufMessage *) s;

  delete self->cpp;
  self->cpp = NULL;

  filterx_object_free_method(s);
}

static gboolean
_truthy(FilterXObject *s)
{
  return TRUE;
}

static gboolean
_marshal(FilterXObject *s, GString *repr, LogMessageValueType *t)
{
  // FilterXProtobufMessage *self = (FilterXProtobufMessage *) s;

  // std::string serialized = self->cpp->marshal();

  // g_string_append_len(repr, serialized.c_str(), serialized.length());
  // *t = LM_VT_PROTOBUF;
  return TRUE;
}

static gboolean
_format_json(FilterXObject *s, GString *json)
{
  // FilterXProtobufMessage *self = (FilterXProtobufMessage *) s;

  // g_string_append_c(json, '{');
  // g_string_append(json, "\"data\":");
  // const std::string data_content = self->cpp->get_value().data();
  // bytes_format_json(data_content.c_str(), data_content.length(), json);

  // g_string_append_c(json, ',');
  // g_string_append(json, "\"attributes\":");
  // g_string_append_c(json, '{');
  // bool first = TRUE;
  // for (const auto &pair : self->cpp->get_value().attributes())
  //   {
  //     const std::string &key = pair.first;
  //     const std::string &value = pair.second;

  //     if (!first)
  //       g_string_append_c(json, ',');
  //     else
  //       first = FALSE;
  //     string_format_json(key.c_str(), key.length(), json);
  //     g_string_append_c(json, ':');
  //     string_format_json(value.c_str(), value.length(), json);
  //   }
  // g_string_append_c(json, '}');
  // g_string_append_c(json, '}');
  return TRUE;
}

FilterXObject *
_filterx_protobuf_message_clone(FilterXObject *s)
{
  // FilterXProtobufMessage *self = (FilterXProtobufMessage *) s;

  // FilterXProtobufMessage *clone = g_new0(FilterXProtobufMessage, 1);
  // _init_instance(clone);

  // try
  //   {
  //     clone->cpp = new Message(*self->cpp, clone);
  //   }
  // catch (const std::runtime_error &)
  //   {
  //     g_assert_not_reached();
  //   }

  // return &clone->super;
  return NULL;
}

static gboolean
_repr(FilterXObject *s, GString *repr)
{
  // FilterXProtobufMessage *self = (FilterXProtobufMessage *) s;

  // try
  //   {
  //     std::string cstring = self->cpp->repr();
  //     g_string_assign(repr, cstring.c_str());
  //   }
  // catch (const std::runtime_error &e)
  //   {
  //     msg_error("FilterX: Failed to repr OTel Logrecord object", evt_tag_str("error", e.what()));
  //     return FALSE;
  //   }
  return TRUE;
}

gboolean
_get_repr(FilterXObject *obj, std::string &str, GString *repr)
{
  const gchar *buf = NULL;
  gsize len;
  if (filterx_object_extract_string_ref(obj, &buf, &len))
    {
      str = std::string(buf, len);
    }
  else
    {
      g_string_truncate(repr, 0);
      if (!filterx_object_str(obj, repr))
        return FALSE;
      str = std::string(repr->str, repr->len);
    }
  return TRUE;
}

FilterXExpr *
filterx_protobuf_message_new_from_args(FilterXFunctionArgs *args, GError **error)
{
  FilterXProtobufMessage *self = g_new0(FilterXProtobufMessage, 1);
  filterx_function_init_instance(&self->super, "protobuf_message");
  return &self->super.super;
}

FILTERX_FUNCTION(protobuf_message, filterx_protobuf_message_new_from_args);

FILTERX_DEFINE_TYPE(protobuf_message, FILTERX_TYPE_NAME(object),
                    .is_mutable = TRUE,
                    .marshal = _marshal,
                    .clone = _filterx_protobuf_message_clone,
                    .truthy = _truthy,
                    .format_json = _format_json,
                    .repr = _repr,
                    .free_fn = _free,
                   );
