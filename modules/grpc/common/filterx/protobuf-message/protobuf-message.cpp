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

#include "protobuf-message.hpp"

#include "compat/cpp-start.h"

#include "filterx/object-extractor.h"
#include "filterx/object-string.h"
#include "filterx/object-datetime.h"
#include "filterx/object-primitive.h"
#include "filterx/expr-literal-generator.h"
#include "scratch-buffers.h"
#include "generic-number.h"

#include "compat/cpp-end.h"

#include <google/protobuf/util/json_util.h>
#include "schema.hpp"

#include <unistd.h>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <limits.h>     // for PATH_MAX

using namespace syslogng::grpc::common;
using namespace syslogng::grpc::common::filterx;
using namespace google::protobuf;
using namespace google::protobuf::compiler;

/* C++ Implementations */

DynamicProtoLoader::DynamicProtoLoader(FilterXProtobufMessage *super_,
  const std::string& message_name,
  const std::map<std::string, std::string>& schema_map)
: super(super_), _schema("example", message_name)
{
// TODO: now use schema_map to add fields to schema
}

DynamicProtoLoader::DynamicProtoLoader(FilterXProtobufMessage *super_,
  const std::string& proto_content,
  const std::string& proto_filename)
: super(super_), _schema(proto_content)
{
// schema is now initialized
}

Schema&
DynamicProtoLoader::getSchema() {
  return _schema;
}

// /* C Wrappers */

#define FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA_FILE "schema_file"
#define FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA "schema"

#define FILTERX_FUNC_PROTOBUF_MESSAGE_USAGE "Usage: protobuf_message({dict}, [" \
FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA"={dict}," \
FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA_FILE"={string literal}])"

static FilterXObject *
_eval(FilterXExpr *s)
{
  FilterXProtobufMessage *self = (FilterXProtobufMessage *) s;

  FilterXObject *data = filterx_expr_eval(self->input);
  FilterXObject *dict = filterx_ref_unwrap_ro(data);

  if (!filterx_object_is_type(dict, &FILTERX_TYPE_NAME(dict))) {
    // TODO: log error
    return NULL;
  }

  auto msg = self->cpp->getSchema().createMessageInstance();
  const Reflection *reflection = msg->GetReflection();
  const Descriptor *descriptor = msg->GetDescriptor();

  // DEBUG
  const FieldDescriptor* id_field = descriptor->FindFieldByName("id");
  reflection->SetInt32(msg.get(), id_field, 447);
  const FieldDescriptor* foobar_field = descriptor->FindFieldByName("foobar");
  if (!foobar_field || foobar_field->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE || !foobar_field->is_repeated()) {
      throw std::runtime_error("Invalid or missing 'foobar' field.");
  }

  const Descriptor* nested_desc = foobar_field->message_type();
  const FieldDescriptor* foo_field = nested_desc->FindFieldByName("foo");
  const FieldDescriptor* bar_field = nested_desc->FindFieldByName("bar");

  if (!foo_field || !bar_field) {
      throw std::runtime_error("Missing 'foo' or 'bar' field in nested message.");
  }

  Message* entry = reflection->AddMessage(msg.get(), foobar_field);
  const Reflection* entry_reflection = entry->GetReflection();
  entry_reflection->SetString(entry, foo_field, "foo1");
  entry_reflection->SetString(entry, bar_field, "bar1");
  Message* entry2 = reflection->AddMessage(msg.get(), foobar_field);
  const Reflection* entry_reflection2 = entry2->GetReflection();
  entry_reflection2->SetString(entry2, foo_field, "foo2");
  entry_reflection2->SetString(entry2, bar_field, "bar2");

  // EO DEBUG

  std::cout << msg->DebugString() << std::endl;

  std::string protobuf_string = msg->SerializePartialAsString();

  return filterx_protobuf_new(protobuf_string.c_str(), protobuf_string.length());
}

static FilterXExpr *
_optimize(FilterXExpr *s)
{
  FilterXProtobufMessage *self = (FilterXProtobufMessage *) s;

  self->input = filterx_expr_optimize(self->input);

  return filterx_function_optimize_method(&self->super);
}

static gboolean
_init(FilterXExpr *s, GlobalConfig *cfg)
{
  FilterXProtobufMessage *self = (FilterXProtobufMessage *) s;

  if (!filterx_expr_init(self->input, cfg))
    return FALSE;

  return filterx_function_init_method(&self->super, cfg);
}

static void
_deinit(FilterXExpr *s, GlobalConfig *cfg)
{
  FilterXProtobufMessage *self = (FilterXProtobufMessage *) s;

  filterx_expr_deinit(self->input, cfg);
  filterx_function_deinit_method(&self->super, cfg);
}

static void
_free(FilterXExpr *s)
{
  FilterXProtobufMessage *self = (FilterXProtobufMessage *) s;

  delete self->cpp;
  self->cpp = NULL;
  filterx_expr_unref(self->input);
  filterx_function_free_method(&self->super);
}

std::string _readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::ostringstream ss;
    ss << file.rdbuf();

    if (file.fail() && !file.eof()) {
        throw std::runtime_error("Error reading from file: " + filename);
    }

    std::string content = ss.str();

    return content;
}

static gboolean
_extract_args(FilterXProtobufMessage *self, FilterXFunctionArgs *args, GError **error)
{
  if (filterx_function_args_len(args) != 1)
    {
      g_set_error(error, FILTERX_FUNCTION_ERROR, FILTERX_FUNCTION_ERROR_CTOR_FAIL,
                  "invalid number of arguments. " FILTERX_FUNC_PROTOBUF_MESSAGE_USAGE);
      return FALSE;
    }

    self->input = filterx_function_args_get_expr(args, 0);
    if (!self->input)
      {
        g_set_error(error, FILTERX_FUNCTION_ERROR, FILTERX_FUNCTION_ERROR_CTOR_FAIL,
                    "input must be set. " FILTERX_FUNC_PROTOBUF_MESSAGE_USAGE);
        return FALSE;
      }

    gboolean exists = FALSE;
    const gchar* proto_filename = filterx_function_args_get_named_literal_string(args, FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA_FILE, NULL, &exists);
    if (exists && proto_filename != NULL) {
      std::string file_name(proto_filename);
      try
        {
          self->cpp = new DynamicProtoLoader(self, _readFile(file_name), file_name);
          self->cpp->getSchema().finalize();
        }
      catch(const std::exception &ex)
        {
          msg_error("protobuf-message: failed to load protobuf:", evt_tag_str("message", ex.what()));
          return FALSE;
        }
    } else {

      FilterXExpr *fx_schema = filterx_function_args_get_named_expr(args, FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA);
      if (!fx_schema) {
        g_set_error(error, FILTERX_FUNCTION_ERROR, FILTERX_FUNCTION_ERROR_CTOR_FAIL,
          "one of '" FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA_FILE "' or '" FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA "' arguments must be set. " FILTERX_FUNC_PROTOBUF_MESSAGE_USAGE);
        return FALSE;
      }

      if (!filterx_expr_is_literal_dict_generator(fx_schema)) {
        g_set_error(error, FILTERX_FUNCTION_ERROR, FILTERX_FUNCTION_ERROR_CTOR_FAIL,
          FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA " must be a dict literal. " FILTERX_FUNC_PROTOBUF_MESSAGE_USAGE);
          return FALSE;
      }

      // if (!filterx_expr_is_literal_list_generator(targets))
      //   {
      //     g_set_error(error, FILTERX_FUNCTION_ERROR, FILTERX_FUNCTION_ERROR_CTOR_FAIL,
      //                 FILTERX_FUNC_UNSET_EMPTIES_ARG_NAME_TARGETS" argument must be literal list. " FILTERX_FUNC_UNSET_EMPTIES_USAGE);
      //     return FALSE;
      //   }

      // FilterXObject *fx_schema = filterx_function_args_get_named_literal_object(args, FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA, &exists);
      // if (!exists) {
      //   g_set_error(error, FILTERX_FUNCTION_ERROR, FILTERX_FUNCTION_ERROR_CTOR_FAIL,
      //     "one of '" FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA_FILE "' or '" FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA "' arguments must be set. " FILTERX_FUNC_PROTOBUF_MESSAGE_USAGE);
      //   return FALSE;
      // }
      // if (!fx_schema) {
      //   g_set_error(error, FILTERX_FUNCTION_ERROR, FILTERX_FUNCTION_ERROR_CTOR_FAIL,
      //     FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA " must be a dict literal. " FILTERX_FUNC_PROTOBUF_MESSAGE_USAGE);
      //     return FALSE;
      // }
      // FilterXObject *dict = filterx_ref_unwrap_ro(fx_schema);
      // if (!filterx_object_is_type(dict, &FILTERX_TYPE_NAME(dict))) {
      //   g_set_error(error, FILTERX_FUNCTION_ERROR, FILTERX_FUNCTION_ERROR_CTOR_FAIL,
      //     FILTERX_FUNC_PROTOBUF_MESSAGE_ARG_NAME_SCHEMA " must be a dict literal. " FILTERX_FUNC_PROTOBUF_MESSAGE_USAGE);
      //   return FALSE;
      // }
      std::map<std::string, std::string> schema;
      self->cpp = new DynamicProtoLoader(self, "User", schema);
    }

    return TRUE;
}

FilterXExpr *
filterx_protobuf_message_new_from_args(FilterXFunctionArgs *args, GError **error)
{
  FilterXProtobufMessage *self = g_new0(FilterXProtobufMessage, 1);
  filterx_function_init_instance(&self->super, "protobuf_message");

  self->super.super.eval = _eval;
  self->super.super.optimize = _optimize;
  self->super.super.init = _init;
  self->super.super.deinit = _deinit;
  self->super.super.free_fn = _free;

  if (!_extract_args(self, args, error) ||
      !filterx_function_args_check(args, error))
    goto error;

  filterx_function_args_free(args);
  return &self->super.super;
error:
  filterx_function_args_free(args);
  filterx_expr_unref(&self->super.super);
  return NULL;
}

FILTERX_FUNCTION(protobuf_message, filterx_protobuf_message_new_from_args);
