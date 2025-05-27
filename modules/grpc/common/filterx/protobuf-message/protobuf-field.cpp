/*
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

#include "protobuf-field.hpp"

#include "compat/cpp-start.h"

#include "filterx/object-extractor.h"
#include "filterx/object-string.h"
#include "filterx/object-message-value.h"
#include "filterx/object-datetime.h"
#include "filterx/object-primitive.h"
#include "scratch-buffers.h"
#include "generic-number.h"
#include "filterx/object-list-interface.h"
#include "filterx/object-dict-interface.h"
#include "filterx/json-repr.h"
#include "filterx/object-null.h"
#include "compat/cpp-end.h"

#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <memory>

using namespace syslogng::grpc::common;
using namespace google::protobuf;
using namespace google::protobuf::compiler;

#define FILTERX_OBJECT_ADDER \
  bool FilterXObjectAdder(Message *message,  \
                          ProtoReflectors reflectors,  \
                          FilterXObject *object,  \
                          FilterXObject **assoc_object) \
  { \
    try \
    { \
      gpointer user_data[] = {&reflectors, message}; \
      guint64 len; \
      filterx_object_len(object, &len); \
      if (len == 0) \
        return TRUE; \
      for (gint64 i = 0; i < ((gint64) len); i++) \
        { \
          FilterXObject *elem = filterx_list_get_subscript(object, i); \
          FilterXObject *elem_unwrapped = filterx_ref_unwrap_ro(elem); \
          iter(NULL, elem_unwrapped, user_data); \
          filterx_object_unref(elem); \
        } \
    } \
    catch (const std::exception &e) \
    { \
      log_type_error(reflectors, object->type->name); \
      return false; \
    } \
    return true; \
  } \


#define GET_USER_DATA                                                                             \
    ProtoReflectors *reflectors = static_cast<ProtoReflectors*>(((gpointer *)user_data)[0]);      \
    auto *message = static_cast<Message*>(((gpointer *)user_data)[1]);          \


void
log_type_error(ProtoReflectors reflectors, const char *type)
{
  msg_error("protobuf-field: Failed to convert field, type is unsupported",
            evt_tag_str("field", reflectors.fieldDescriptor->name().data()),
            evt_tag_str("expected_type", reflectors.field_type_name()),
            evt_tag_str("type", type));
}

float
double_to_float_safe(double val)
{
  if (val < (double)(-FLT_MAX))
    return -FLT_MAX;
  else if (val > (double)(FLT_MAX))
    return FLT_MAX;
  return (float)val;
}

static gboolean field_iterator(FilterXObject *key, FilterXObject *value, gpointer user_data)
{
  try
    {
      auto *message = static_cast<Message*>(((gpointer *)user_data)[0]);

      std::string field_name = extract_string_from_object(key);
      ProtoReflectors sub_reflectors(*message, field_name);

      ProtobufField *pbf = protobuf_converter_by_type((sub_reflectors).fieldType);

      FilterXObject *assoc_object = NULL;
      if (!pbf->Set(message, field_name, value, &assoc_object))
          return FALSE;
      filterx_object_unref(value);
      value = assoc_object;
    }
  catch (const std::exception &e)
    {
      msg_error("field iteration error",
                evt_tag_str("error", e.what()));
      return FALSE;
    }

  return TRUE;
}


/* C++ Implementations */

// ProtoReflectors reflectors

bool ProtobufField::add(Message *message, const std::string &fieldName, FilterXObject *object,
                        FilterXObject **assoc_object)
{
  try
    {
      ProtoReflectors reflectors(*message, fieldName);
      FilterXObject *array = filterx_ref_unwrap_ro(object);
      if (!array || (!filterx_object_is_type(array, &FILTERX_TYPE_NAME(list))
                     && !filterx_object_is_type(array, &FILTERX_TYPE_NAME(dict))))
        throw std::runtime_error("ProtoField: Add: not a list object");

      if (this->FilterXObjectAdder(message, reflectors, array, assoc_object))
        {
          if (!(*assoc_object))
            *assoc_object = filterx_object_ref(object);
          return true;
        }
      return false;
    }
  catch(const std::exception &ex)
    {
      msg_error("protobuf-field: Failed to add field:", evt_tag_str("message", ex.what()));
      return false;
    }
}

class BoolField : public ProtobufField
{
private:
  static gboolean extract(FilterXObject *object)
  {
    return filterx_object_truthy(object);
  }
  static gboolean iter(FilterXObject *key, FilterXObject *value, gpointer user_data)
  {
    ProtoReflectors *reflectors = static_cast<ProtoReflectors *>(((gpointer *)user_data)[0]);
    auto *message = static_cast<Message *>(((gpointer *)user_data)[1]);
    reflectors->reflection->AddBool(message, reflectors->fieldDescriptor, extract(value));
    return TRUE;
  }
public:
  FilterXObject *FilterXObjectGetter(Message *message, ProtoReflectors reflectors)
  {
    return filterx_boolean_new(reflectors.reflection->GetBool(*message, reflectors.fieldDescriptor));
  }
  bool FilterXObjectSetter(Message *message, ProtoReflectors reflectors, FilterXObject *object,
                           FilterXObject **assoc_object)
  {
    reflectors.reflection->SetBool(message, reflectors.fieldDescriptor, extract(object));
    return true;
  }
  FILTERX_OBJECT_ADDER
};

class i32Field : public ProtobufField
{
private:
  static int32_t extract(FilterXObject *object)
  {
    gint64 i;
    if (filterx_object_extract_integer(object, &i))
      return MAX(INT32_MIN, MIN(INT32_MAX, i));
    throw std::runtime_error("ProtoField extract: unsupported type");
  }
  static gboolean iter(FilterXObject *key, FilterXObject *value, gpointer user_data)
  {
    GET_USER_DATA;
    reflectors->reflection->AddInt32(message, reflectors->fieldDescriptor, extract(value));
    return TRUE;
  }
public:
  FilterXObject *FilterXObjectGetter(Message *message, ProtoReflectors reflectors)
  {
    return filterx_integer_new(gint32(reflectors.reflection->GetInt32(*message, reflectors.fieldDescriptor)));
  }
  bool FilterXObjectSetter(Message *message, ProtoReflectors reflectors, FilterXObject *object,
                           FilterXObject **assoc_object)
  {
    try
      {
        reflectors.reflection->SetInt32(message, reflectors.fieldDescriptor, extract(object));
      }
    catch(const std::exception &e)
      {
        log_type_error(reflectors, object->type->name);
        return false;
      }
    return true;
  }
  FILTERX_OBJECT_ADDER
};

class i64Field : public ProtobufField
{
private:
  static int64_t extract(FilterXObject *object)
  {
    gint64 i;
    if (filterx_object_extract_integer(object, &i))
      return i;

    UnixTime utime;
    if (filterx_object_extract_datetime(object, &utime))
      {
        uint64_t unix_epoch = unix_time_to_unix_epoch_usec(utime);
        return (int64_t)(unix_epoch);
      }

    throw std::runtime_error("ProtoField extract: unsupported type");
  }
  static gboolean iter(FilterXObject *key, FilterXObject *value, gpointer user_data)
  {
    GET_USER_DATA;
    reflectors->reflection->AddInt64(message, reflectors->fieldDescriptor, extract(value));
    return TRUE;
  }
public:
  FilterXObject *FilterXObjectGetter(Message *message, ProtoReflectors reflectors)
  {
    return filterx_integer_new(gint64(reflectors.reflection->GetInt64(*message, reflectors.fieldDescriptor)));
  }
  bool FilterXObjectSetter(Message *message, ProtoReflectors reflectors, FilterXObject *object,
                           FilterXObject **assoc_object)
  {
    try
      {
        reflectors.reflection->SetInt64(message, reflectors.fieldDescriptor, extract(object));
      }
    catch(const std::exception &e)
      {
        log_type_error(reflectors, object->type->name);
        return false;
      }

    return true;
  }
  FILTERX_OBJECT_ADDER
};

class u32Field : public ProtobufField
{
private:
  static uint32_t extract(FilterXObject *object)
  {
    gint64 i;
    if (filterx_object_extract_integer(object, &i))
      return MAX(0, MIN(UINT32_MAX, i));

    throw std::runtime_error("ProtoField extract: unsupported type");
  }
  static gboolean iter(FilterXObject *key, FilterXObject *value, gpointer user_data)
  {
    GET_USER_DATA;
    reflectors->reflection->AddUInt32(message, reflectors->fieldDescriptor, extract(value));
    return TRUE;
  }
public:
  FilterXObject *FilterXObjectGetter(Message *message, ProtoReflectors reflectors)
  {
    return filterx_integer_new(guint32(reflectors.reflection->GetUInt32(*message, reflectors.fieldDescriptor)));
  }
  bool FilterXObjectSetter(Message *message, ProtoReflectors reflectors, FilterXObject *object,
                           FilterXObject **assoc_object)
  {
    try
      {
        reflectors.reflection->SetUInt32(message, reflectors.fieldDescriptor, extract(object));
      }
    catch(const std::exception &e)
      {
        log_type_error(reflectors, object->type->name);
        return false;
      }
    return true;
  }
  FILTERX_OBJECT_ADDER
};

class u64Field : public ProtobufField
{
private:
  static uint64_t extract(FilterXObject *object)
  {
    gint64 i;
    if (filterx_object_extract_integer(object, &i))
      return MAX(0, MIN(UINT64_MAX, (uint64_t) i));

    UnixTime utime;
    if (filterx_object_extract_datetime(object, &utime))
      return unix_time_to_unix_epoch_usec(utime);

    throw std::runtime_error("ProtoField extract: unsupported type");
  }
  static gboolean iter(FilterXObject *key, FilterXObject *value, gpointer user_data)
  {
    GET_USER_DATA;
    reflectors->reflection->AddUInt64(message, reflectors->fieldDescriptor, extract(value));
    return TRUE;
  }
public:
  FilterXObject *FilterXObjectGetter(Message *message, ProtoReflectors reflectors)
  {
    uint64_t val = reflectors.reflection->GetUInt64(*message, reflectors.fieldDescriptor);
    if (val > INT64_MAX)
      {
        msg_error("protobuf-field: exceeding FilterX number value range",
                  evt_tag_str("field", reflectors.fieldDescriptor->name().data()),
                  evt_tag_long("range_min", INT64_MIN),
                  evt_tag_long("range_max", INT64_MAX),
                  evt_tag_printf("current", "%" G_GUINT64_FORMAT, val));
        return NULL;
      }
    return filterx_integer_new(guint64(val));
  }
  bool FilterXObjectSetter(Message *message, ProtoReflectors reflectors, FilterXObject *object,
                           FilterXObject **assoc_object)
  {
    try
      {
        reflectors.reflection->SetUInt64(message, reflectors.fieldDescriptor, extract(object));
      }
    catch(const std::exception &e)
      {
        log_type_error(reflectors, object->type->name);
        return false;
      }
    return true;
  }
  FILTERX_OBJECT_ADDER
};

class StringField : public ProtobufField
{
private:
  static std::string extract(FilterXObject *object)
  {
    const gchar *str;
    gsize len;

    if (filterx_object_extract_string_ref(object, &str, &len))
      return std::string(str, len);

    if (filterx_object_is_type(object, &FILTERX_TYPE_NAME(message_value)) &&
        filterx_message_value_get_type(object) == LM_VT_JSON)
      {
        str = filterx_message_value_get_value(object, &len);
        return std::string(str, len);
      }

    object = filterx_ref_unwrap_ro(object);
    if (filterx_object_is_type(object, &FILTERX_TYPE_NAME(dict)) ||
        filterx_object_is_type(object, &FILTERX_TYPE_NAME(list)))
      {
        GString *repr = scratch_buffers_alloc();

        if (!filterx_object_to_json(object, repr))
          throw std::runtime_error("ProtoField extract: json marshal error");
        len = repr->len;
        str = repr->str;
        return std::string(str, len);
      }

    throw std::runtime_error("ProtoField extract: unsupported type");
  }
  static gboolean iter(FilterXObject *key, FilterXObject *value, gpointer user_data)
  {
    GET_USER_DATA;
    reflectors->reflection->AddString(message, reflectors->fieldDescriptor, extract(value));
    return TRUE;
  }
public:
  FilterXObject *FilterXObjectGetter(Message *message, ProtoReflectors reflectors)
  {
    std::string bytesBuffer;
    const std::string &bytesRef = reflectors.reflection->GetStringReference(*message, reflectors.fieldDescriptor,
                                  &bytesBuffer);
    return filterx_string_new(bytesRef.c_str(), bytesRef.length());
  }
  bool FilterXObjectSetter(Message *message, ProtoReflectors reflectors, FilterXObject *object,
                           FilterXObject **assoc_object)
  {
    try
      {
        reflectors.reflection->SetString(message, reflectors.fieldDescriptor, extract(object));
      }
    catch(const std::exception &e)
      {
        log_type_error(reflectors, object->type->name);
        return false;
      }

    return true;
  }
  bool FilterXObjectAdder(Message *message, ProtoReflectors reflectors, FilterXObject *object, FilterXObject **assoc_object)
  {
    try
    {
      gpointer user_data[] = {&reflectors, message};
      guint64 len;
      filterx_object_len(object, &len);
      if (len == 0)
        return TRUE;
      for (gint64 i = 0; i < ((gint64) len); i++)
        {
          FilterXObject *elem = filterx_list_get_subscript(object, i);
          FilterXObject *elem_unwrapped = filterx_ref_unwrap_ro(elem);
          iter(NULL, elem_unwrapped, user_data);
          filterx_object_unref(elem);
        }
    }
    catch (const std::exception &e)
    {
      log_type_error(reflectors, object->type->name);
      return false;
    }
    return true;
  }
};

class DoubleField : public ProtobufField
{
private:
  static double extract(FilterXObject *object)
  {
    gint64 i;
    if (filterx_object_extract_integer(object, &i))
      return i;

    gdouble d;
    if (filterx_object_extract_double(object, &d))
      return d;

    throw std::runtime_error("ProtoField extract: unsupported type");
  }
  static gboolean iter(FilterXObject *key, FilterXObject *value, gpointer user_data)
  {
    GET_USER_DATA;
    reflectors->reflection->AddDouble(message, reflectors->fieldDescriptor, extract(value));
    return TRUE;
  }
public:
  FilterXObject *FilterXObjectGetter(Message *message, ProtoReflectors reflectors)
  {
    return filterx_double_new(gdouble(reflectors.reflection->GetDouble(*message, reflectors.fieldDescriptor)));
  }
  bool FilterXObjectSetter(Message *message, ProtoReflectors reflectors, FilterXObject *object,
                           FilterXObject **assoc_object)
  {
    try
      {
        reflectors.reflection->SetDouble(message, reflectors.fieldDescriptor, extract(object));
      }
    catch(const std::exception &e)
      {
        log_type_error(reflectors, object->type->name);
        return false;
      }
    return true;
  }
  FILTERX_OBJECT_ADDER
};

class FloatField : public ProtobufField
{
private:
  static float extract(FilterXObject *object)
  {
    gint64 i;
    if (filterx_object_extract_integer(object, &i))
      return double_to_float_safe(i);

    gdouble d;
    if (filterx_object_extract_double(object, &d))
      return double_to_float_safe(d);

    throw std::runtime_error("ProtoField extract: unsupported type");
  }
  static gboolean iter(FilterXObject *key, FilterXObject *value, gpointer user_data)
  {
    GET_USER_DATA;
    reflectors->reflection->AddFloat(message, reflectors->fieldDescriptor, extract(value));
    return TRUE;
  }
public:
  FilterXObject *FilterXObjectGetter(Message *message, ProtoReflectors reflectors)
  {
    return filterx_double_new(gdouble(reflectors.reflection->GetFloat(*message, reflectors.fieldDescriptor)));
  }
  bool FilterXObjectSetter(Message *message, ProtoReflectors reflectors, FilterXObject *object,
                           FilterXObject **assoc_object)
  {
    try
      {
        reflectors.reflection->SetDouble(message, reflectors.fieldDescriptor, extract(object));
      }
    catch(const std::exception &e)
      {
        log_type_error(reflectors, object->type->name);
        return false;
      }
    return true;
  }
  FILTERX_OBJECT_ADDER
};

class BytesField : public ProtobufField
{
private:
  static std::string extract(FilterXObject *object)
  {
    const gchar *str;
    gsize len;

    if (filterx_object_extract_bytes_ref(object, &str, &len) ||
        filterx_object_extract_protobuf_ref(object, &str, &len))
      return std::string(str, len);

    throw std::runtime_error("ProtoField extract: unsupported type");
  }
  static gboolean iter(FilterXObject *key, FilterXObject *value, gpointer user_data)
  {
    GET_USER_DATA;
    reflectors->reflection->AddString(message, reflectors->fieldDescriptor, extract(value));
    return TRUE;
  }
public:
  FilterXObject *FilterXObjectGetter(Message *message, ProtoReflectors reflectors)
  {
    std::string bytesBuffer;
    const std::string &bytesRef = reflectors.reflection->GetStringReference(*message, reflectors.fieldDescriptor,
                                  &bytesBuffer);
    return filterx_bytes_new(bytesRef.c_str(), bytesRef.length());
  }
  bool FilterXObjectSetter(Message *message, ProtoReflectors reflectors, FilterXObject *object,
                           FilterXObject **assoc_object)
  {
    try
      {
        reflectors.reflection->SetString(message, reflectors.fieldDescriptor, extract(object));
      }
    catch(const std::exception &e)
      {
        log_type_error(reflectors, object->type->name);
        return false;
      }
    return true;
  }
  FILTERX_OBJECT_ADDER
};

class MapField : public ProtobufField
{
private:
  static gboolean iter(FilterXObject *key, FilterXObject *value, gpointer user_data)
  {
    try
      {
        const gchar *key_name = "key";
        const gchar *val_name = "value";
        GET_USER_DATA;
        Message *entry_message = reflectors->reflection->AddMessage(message, reflectors->fieldDescriptor);
        ProtoReflectors key_reflectors(*entry_message, key_name);
        ProtoReflectors val_reflectors(*entry_message, val_name);

        ProtobufField *pbf_key = protobuf_converter_by_type(key_reflectors.fieldType);
        ProtobufField *pbf_val = protobuf_converter_by_type(val_reflectors.fieldType);

        FilterXObject *assoc_key_object = NULL;
        if (!pbf_key->Set(entry_message, key_name, key, &assoc_key_object))
          {
            return FALSE;
          };
        filterx_object_unref(key);
        key = assoc_key_object;
        FilterXObject *assoc_val_object = NULL;
        if (!pbf_val->Set(entry_message, val_name, value, &assoc_val_object))
          {
            return FALSE;
          };
        filterx_object_unref(value);
        value = assoc_val_object;
      }
    catch (const std::exception &e)
      {
        msg_error("dict iteration error",
                  evt_tag_str("error", e.what()));
        return FALSE;
      }

    return TRUE;
  }
public:
  FilterXObject *FilterXObjectGetter(Message *message, ProtoReflectors reflectors)
  {
    throw std::logic_error("not yet implemented");
    return NULL;
  }
  bool FilterXObjectSetter(Message *message, ProtoReflectors reflectors, FilterXObject *object,
                           FilterXObject **assoc_object)
  {
    return true;
  }
  bool FilterXObjectAdder(Message *message,
                          ProtoReflectors reflectors,
                          FilterXObject *object,
                          FilterXObject **assoc_object)
  {
    try
      {
        gpointer user_data[] = {&reflectors, message};
        return filterx_dict_iter(object, iter, user_data);
      }
    catch (const std::exception &e)
      {
        log_type_error(reflectors, object->type->name);
        return false;
      }
    return true;
  }
};

class RepeatedNested : public ProtobufField
{
private:
static gboolean iter(FilterXObject *key, FilterXObject *value, gpointer user_data)
{
  GET_USER_DATA;

  Message* sub_msg = reflectors->reflection->AddMessage(message, reflectors->fieldDescriptor);

  gpointer sub_user_data[] = {sub_msg};

  FilterXObject *dict = filterx_ref_unwrap_ro(value);
  if (!dict || !filterx_object_is_type(dict, &FILTERX_TYPE_NAME(dict)))
      throw std::runtime_error("ProtoField: Add: not a list object");

  return filterx_dict_iter(dict, field_iterator, sub_user_data);
}
public:
  FilterXObject *FilterXObjectGetter(Message *message, ProtoReflectors reflectors)
  {
    throw std::logic_error("not yet implemented");
    return NULL;
  }
  bool FilterXObjectSetter(Message *message, ProtoReflectors reflectors, FilterXObject *object,
                           FilterXObject **assoc_object)
  {
    return true;
  }
  FILTERX_OBJECT_ADDER
};


class MessageField : public ProtobufField
{
public:
  FilterXObject *FilterXObjectGetter(Message *message, ProtoReflectors reflectors)
  {
    throw std::logic_error("ProtobufField: not yet implemented");
    return NULL;
  }
  bool FilterXObjectSetter(Message *message, ProtoReflectors reflectors, FilterXObject *object,
                           FilterXObject **assoc_object)
  {
    Message* sub_msg = reflectors.reflection->MutableMessage(message, reflectors.fieldDescriptor);

    gpointer user_data[] = {sub_msg};

    FilterXObject *dict = filterx_ref_unwrap_ro(object);
    if (!dict || !filterx_object_is_type(dict, &FILTERX_TYPE_NAME(dict)))
        throw std::runtime_error("ProtoField: Add: not a list object");

    return filterx_dict_iter(dict, field_iterator, user_data);
  }
  bool FilterXObjectAdder(Message *message, ProtoReflectors reflectors, FilterXObject *object,
                          FilterXObject **assoc_object)
  {
    std::unique_ptr<ProtobufField> inner;
    if (reflectors.fieldDescriptor->is_map())
      {
        inner = std::make_unique<MapField>();
      }
      else if (reflectors.fieldDescriptor->is_repeated())
      {
        inner = std::make_unique<RepeatedNested>();
      }
    if (!inner)
      {
        throw std::logic_error("ProtobufField: unknown TYPE_MESSAGE handler");
      }
    return inner->FilterXObjectAdder(message, reflectors, object, assoc_object);
  }
};

std::unique_ptr<ProtobufField> *syslogng::grpc::common::all_protobuf_converters()
{
  static std::unique_ptr<ProtobufField> Converters[google::protobuf::FieldDescriptor::MAX_TYPE] =
  {
    std::make_unique<DoubleField>(),  //TYPE_DOUBLE = 1,    // double, exactly eight bytes on the wire.
    std::make_unique<FloatField>(),   //TYPE_FLOAT = 2,     // float, exactly four bytes on the wire.
    std::make_unique<i64Field>(),     //TYPE_INT64 = 3,     // int64, varint on the wire.  Negative numbers
    // take 10 bytes.  Use TYPE_SINT64 if negative
    // values are likely.
    std::make_unique<u64Field>(),     //TYPE_UINT64 = 4,    // uint64, varint on the wire.
    std::make_unique<i32Field>(),     //TYPE_INT32 = 5,     // int32, varint on the wire.  Negative numbers
    // take 10 bytes.  Use TYPE_SINT32 if negative
    // values are likely.
    std::make_unique<u64Field>(),     //TYPE_FIXED64 = 6,   // uint64, exactly eight bytes on the wire.
    std::make_unique<u32Field>(),     //TYPE_FIXED32 = 7,   // uint32, exactly four bytes on the wire.
    std::make_unique<BoolField>(),    //TYPE_BOOL = 8,      // bool, varint on the wire.
    std::make_unique<StringField>(),  //TYPE_STRING = 9,    // UTF-8 text.
    nullptr,                          //TYPE_GROUP = 10,    // Tag-delimited message.  Deprecated.
    std::make_unique<MessageField>(), //TYPE_MESSAGE = 11,  // Length-delimited message.
    std::make_unique<BytesField>(),   //TYPE_BYTES = 12,     // Arbitrary byte array.
    std::make_unique<u32Field>(),     //TYPE_UINT32 = 13,    // uint32, varint on the wire
    nullptr,                          //TYPE_ENUM = 14,      // Enum, varint on the wire
    std::make_unique<i32Field>(),     //TYPE_SFIXED32 = 15,  // int32, exactly four bytes on the wire
    std::make_unique<i64Field>(),     //TYPE_SFIXED64 = 16,  // int64, exactly eight bytes on the wire
    std::make_unique<i32Field>(),     //TYPE_SINT32 = 17,    // int32, ZigZag-encoded varint on the wire
    std::make_unique<i64Field>(),     //TYPE_SINT64 = 18,    // int64, ZigZag-encoded varint on the wire
  };
  return Converters;
};

ProtobufField *syslogng::grpc::common::protobuf_converter_by_type(google::protobuf::FieldDescriptor::Type fieldType)
{
  g_assert(fieldType <= google::protobuf::FieldDescriptor::MAX_TYPE && fieldType > 0);
  return all_protobuf_converters()[fieldType - 1].get();
}

std::string
syslogng::grpc::common::extract_string_from_object(FilterXObject *object)
{
  const gchar *key_c_str;
  gsize len;

  if (!filterx_object_extract_string_ref(object, &key_c_str, &len))
    throw std::runtime_error("not a string instance");

  return std::string{key_c_str, len};
}

uint64_t
syslogng::grpc::common::get_protobuf_message_set_field_count(const Message &message)
{
  const google::protobuf::Reflection *reflection = message.GetReflection();
  std::vector<const google::protobuf::FieldDescriptor *> fields;
  reflection->ListFields(message, &fields);
  return fields.size();
}
