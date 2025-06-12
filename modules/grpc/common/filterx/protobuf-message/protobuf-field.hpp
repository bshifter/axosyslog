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

#ifndef COMMON_PROTOBUF_FIELD_HPP
#define COMMON_PROTOBUF_FIELD_HPP

#include "syslog-ng.h"

#include "compat/cpp-start.h"
#include "filterx/filterx-object.h"
#include "compat/cpp-end.h"

// #include "opentelemetry/proto/logs/v1/logs.pb.h"
#include "google/protobuf/message.h"

#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>

namespace syslogng {
namespace grpc {
namespace common {

struct ProtoReflectors
{
  const google::protobuf::Reflection *reflection;
  const google::protobuf::Descriptor *descriptor;
  const google::protobuf::FieldDescriptor *fieldDescriptor;
  google::protobuf::FieldDescriptor::Type fieldType;
  ProtoReflectors(const google::protobuf::Message &message, const std::string &fieldName)
  {
    this->reflection = message.GetReflection();
    this->descriptor = message.GetDescriptor();
    if (!this->reflection || !this->descriptor)
      {
        std::string error_msg = "unable to access reflector for protobuf message: "
                                + std::string(message.GetTypeName());
        throw std::invalid_argument(error_msg);
      }
    this->fieldDescriptor = this->descriptor->FindFieldByName(fieldName);
    if (!this->fieldDescriptor)
      {
        std::string error_msg = "unknown field name: " + fieldName;
        throw std::invalid_argument(error_msg);
      }
    this->fieldType = this->fieldDescriptor->type();
    if (this->fieldType >= google::protobuf::FieldDescriptor::MAX_TYPE ||
        this->fieldType < 1)
      {
        std::string error_msg = "unknown field type: " + fieldName + ", " +  std::to_string(this->fieldType);
        throw std::invalid_argument(error_msg);
      }
  };

  const char *
  field_type_name() const
  {
#if GOOGLE_PROTOBUF_VERSION >= 6030000
    return this->fieldDescriptor->type_name().data();
#else
    return this->fieldDescriptor->type_name();
#endif
  }
};

class ProtobufField
{
public:
  FilterXObject *Get(google::protobuf::Message *message, const std::string &fieldName)
  {
    try
      {
        ProtoReflectors reflectors(*message, fieldName);
        return this->FilterXObjectGetter(message, reflectors);
      }
    catch(const std::exception &ex)
      {
        msg_error("protobuf-field: Failed to get field:", evt_tag_str("message", ex.what()));
        return nullptr;
      }
  };
  bool Set(google::protobuf::Message *message, const std::string &fieldName, FilterXObject *object,
           FilterXObject **assoc_object)
  {
    try
      {
        ProtoReflectors reflectors(*message, fieldName);
        if (this->FilterXObjectSetter(message, reflectors, object, assoc_object))
          {
            if (!(*assoc_object))
              *assoc_object = filterx_object_ref(object);
            return true;
          }
        return false;
      }
    catch(const std::exception &ex)
      {
        msg_error("protobuf-field: Failed to set field:", evt_tag_str("message", ex.what()));
        return false;
      }
  }
  bool Unset(google::protobuf::Message *message, const std::string &fieldName)
  {
    try
      {
        ProtoReflectors reflectors(*message, fieldName);
        reflectors.reflection->ClearField(message, reflectors.fieldDescriptor);
        return true;
      }
    catch(const std::exception &ex)
      {
        msg_error("protobuf-field: Failed to unset field:", evt_tag_str("message", ex.what()));
        return false;
      }
  }
  bool IsSet(google::protobuf::Message *message, const std::string &fieldName)
  {
    try
      {
        ProtoReflectors reflectors(*message, fieldName);
        return reflectors.reflection->HasField(*message, reflectors.fieldDescriptor);
      }
    catch(const std::exception &ex)
      {
        msg_error("protobuf-field: Failed to check field:", evt_tag_str("message", ex.what()));
        return false;
      }
  }

  virtual ~ProtobufField() {}
// protected:
  virtual FilterXObject *FilterXObjectGetter(google::protobuf::Message *message, ProtoReflectors reflectors) = 0;
  virtual bool FilterXObjectSetter(google::protobuf::Message *message, ProtoReflectors reflectors,
                                   FilterXObject *object, FilterXObject **assoc_object) = 0;
};

std::unique_ptr<ProtobufField> *all_protobuf_converters();
ProtobufField *protobuf_converter_by_type(google::protobuf::FieldDescriptor::Type fieldType);

std::string extract_string_from_object(FilterXObject *object);

uint64_t get_protobuf_message_set_field_count(const google::protobuf::Message &message);

}
}
}

#endif
