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
 *
 */

#include "object-otel-kvlist.hpp"
#include "otel-field.hpp"

#include "compat/cpp-start.h"
#include "filterx/object-string.h"
#include "compat/cpp-end.h"

#include <stdexcept>

using namespace syslogng::grpc::otel::filterx;
using opentelemetry::proto::common::v1::KeyValue;
using opentelemetry::proto::common::v1::KeyValueList;

/* C++ Implementations */

KVList::KVList(FilterXOtelKVList *s) : super(s)
{
}

KVList::KVList(FilterXOtelKVList *s, FilterXObject *protobuf_object) : super(s)
{
  gsize length;
  const gchar *value = filterx_protobuf_get_value(protobuf_object, &length);

  if (!value)
    throw std::runtime_error("Argument is not a protobuf object");

  if (!kvlist.ParsePartialFromArray(value, length))
    throw std::runtime_error("Failed to parse from protobuf object");
}

KVList::KVList(const KVList &o, FilterXOtelKVList *s) : super(s), kvlist(o.kvlist)
{
}

std::string
KVList::marshal(void)
{
  return kvlist.SerializePartialAsString();
}

opentelemetry::proto::common::v1::KeyValue *
KVList::get_mutable_kv_for_key(const char *key)
{
  for (int i = 0; i < kvlist.values_size(); i++)
    {
      KeyValue *possible_kv = kvlist.mutable_values(i);

      if (possible_kv->key().compare(key) == 0)
        return possible_kv;
    }

  return kvlist.add_values();
}

const opentelemetry::proto::common::v1::KeyValue &
KVList::get_kv_for_key(const char *key)
{
  for (const KeyValue &possible_kv : kvlist.values())
    {
      if (possible_kv.key().compare(key) == 0)
        return possible_kv;
    }

  throw std::out_of_range(std::string("Key is not found: ") + key);
}

bool
KVList::set_subscript(FilterXObject *key, FilterXObject *value)
{
  const gchar *key_c_str = filterx_string_get_value(key, NULL);
  if (!key_c_str)
    {
      msg_error("FilterX: Failed to set OTel KVList element",
                evt_tag_str("error", "Key must be string type"));
      return false;
    }

  ProtobufField *converter = otel_converter_by_type(FieldDescriptor::TYPE_MESSAGE);

  KeyValue *kv = get_mutable_kv_for_key(key_c_str);
  return converter->Set(kv, "value", value);
}

FilterXObject *
KVList::get_subscript(FilterXObject *key)
{
  const gchar *key_c_str = filterx_string_get_value(key, NULL);
  if (!key_c_str)
    {
      msg_error("FilterX: Failed to get OTel KVList element",
                evt_tag_str("error", "Key must be string type"));
      return NULL;
    }

  ProtobufField *converter = otel_converter_by_type(FieldDescriptor::TYPE_MESSAGE);

  try
    {
      const KeyValue &kv = get_kv_for_key(key_c_str);
      return converter->Get(kv, "value");
    }
  catch (const std::out_of_range &e)
    {
      msg_error("FilterX: Failed to get OTel KVList element",
                evt_tag_str("key", key_c_str),
                evt_tag_str("error", e.what()));
      return NULL;
    }
}

const KeyValueList &
KVList::get_value() const
{
  return kvlist;
}

/* C Wrappers */

FilterXObject *
_filterx_otel_kvlist_clone(FilterXObject *s)
{
  FilterXOtelKVList *self = (FilterXOtelKVList *) s;

  FilterXOtelKVList *clone = g_new0(FilterXOtelKVList, 1);
  filterx_object_init_instance(&clone->super, &FILTERX_TYPE_NAME(otel_kvlist));

  try
    {
      clone->cpp = new KVList(*self->cpp, self);
    }
  catch (const std::runtime_error &)
    {
      g_assert_not_reached();
    }

  return &clone->super;
}

static void
_free(FilterXObject *s)
{
  FilterXOtelKVList *self = (FilterXOtelKVList *) s;

  delete self->cpp;
  self->cpp = NULL;
}

static gboolean
_set_subscript(FilterXObject *s, FilterXObject *key, FilterXObject *new_value)
{
  FilterXOtelKVList *self = (FilterXOtelKVList *) s;

  return self->cpp->set_subscript(key, new_value);
}

static FilterXObject *
_get_subscript(FilterXObject *s, FilterXObject *key)
{
  FilterXOtelKVList *self = (FilterXOtelKVList *) s;

  return self->cpp->get_subscript(key);
}

static gboolean
_truthy(FilterXObject *s)
{
  return TRUE;
}

static gboolean
_marshal(FilterXObject *s, GString *repr, LogMessageValueType *t)
{
  FilterXOtelKVList *self = (FilterXOtelKVList *) s;

  std::string serialized = self->cpp->marshal();

  g_string_truncate(repr, 0);
  g_string_append_len(repr, serialized.c_str(), serialized.length());
  *t = LM_VT_PROTOBUF;
  return TRUE;
}

FilterXObject *
otel_kvlist_new(GPtrArray *args)
{
  FilterXOtelKVList *s = g_new0(FilterXOtelKVList, 1);
  filterx_object_init_instance((FilterXObject *)s, &FILTERX_TYPE_NAME(otel_kvlist));

  try
    {
      if (!args || args->len == 0)
        s->cpp = new KVList(s);
      else if (args->len == 1)
        s->cpp = new KVList(s, (FilterXObject *) g_ptr_array_index(args, 0));
      else
        throw std::runtime_error("Invalid number of arguments");
    }
  catch (const std::runtime_error &e)
    {
      msg_error("FilterX: Failed to create OTel KVList object", evt_tag_str("error", e.what()));
      filterx_object_unref(&s->super);
      return NULL;
    }

  return &s->super;
}

gpointer
grpc_otel_filterx_kvlist_construct_new(Plugin *self)
{
  return (gpointer) &otel_kvlist_new;
}

FILTERX_DEFINE_TYPE(otel_kvlist, FILTERX_TYPE_NAME(object),
                    .is_mutable = TRUE,
                    .marshal = _marshal,
                    .clone = _filterx_otel_kvlist_clone,
                    .truthy = _truthy,
                    .get_subscript = _get_subscript,
                    .set_subscript = _set_subscript,
                    .free_fn = _free,
                   );
