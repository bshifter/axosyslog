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

#include <criterion/criterion.h>
#include <criterion/parameterized.h>

#include "apphook.h"
#include "string-list.h"
#include "cfg.h"
#include "plugin.h"
#include "scratch-buffers.h"

#include "schema.hpp"

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

using namespace syslogng::grpc::common;

Test(schema, empty)
{
  Schema builder("my.pkg", "MyMessage");

  std::string actual(builder.getProtoAsString());

  const auto &fileProto = builder.getFileProto();

  cr_expect_str_eq(fileProto.name().c_str(), "generated.proto");
  cr_expect_str_eq(fileProto.package().c_str(), "my.pkg");

  cr_expect_eq(fileProto.message_type_size(), 1, "Expected 1 message type");

  const auto &msg = fileProto.message_type(0);
  cr_expect_str_eq(msg.name().c_str(), "MyMessage");

}

Test(schema, add_single_field)
{
  Schema builder("my.pkg", "MyMessage");
  google::protobuf::FieldDescriptorProto field1;
  field1.set_name("id");
  field1.set_number(1);
  field1.set_type(google::protobuf::FieldDescriptorProto::TYPE_INT32);
  field1.set_label(google::protobuf::FieldDescriptorProto::LABEL_OPTIONAL);
  builder.addField(field1);
  const auto &fileProto = builder.getFileProto();
  const auto &msg = fileProto.message_type(0);
  cr_assert_eq(msg.field_size(), 1, "Expected 1 field");

  const auto &field = msg.field(0);
  cr_assert_str_eq(field.name().c_str(), "id");
  cr_assert_eq(field.type(), google::protobuf::FieldDescriptorProto::TYPE_INT32);
}

Test(schema, message_instance)
{
  Schema builder("my.pkg", "MyMessage");
  google::protobuf::FieldDescriptorProto field1;
  field1.set_name("id");
  field1.set_number(1);
  field1.set_type(google::protobuf::FieldDescriptorProto::TYPE_INT32);
  field1.set_label(google::protobuf::FieldDescriptorProto::LABEL_OPTIONAL);
  builder.addField(field1);

  builder.finalize();
  auto msg = builder.createMessageInstance();

  const auto *descriptor = msg->GetDescriptor();
  // const auto* reflection = msg->GetReflection();
  const auto *field = descriptor->FindFieldByName("id");

  cr_assert_not_null(field);
  cr_assert_eq(field->type(), google::protobuf::FieldDescriptorProto::TYPE_INT32);
}

Test(schema, nested_messages)
{
  Schema builder("my.pkg", "MyMessage");
  google::protobuf::FieldDescriptorProto field1;
  field1.set_name("id");
  field1.set_number(1);
  field1.set_type(google::protobuf::FieldDescriptorProto::TYPE_INT32);
  builder.addField(field1);

  google::protobuf::DescriptorProto desc;

  desc.set_name("nested");

  google::protobuf::FieldDescriptorProto field2;
  field2.set_name("foo");
  field2.set_number(2);
  field2.set_type(google::protobuf::FieldDescriptorProto::TYPE_STRING);
  *desc.add_field() = field2;
  google::protobuf::FieldDescriptorProto field3;
  field3.set_name("bar");
  field3.set_number(3);
  field3.set_type(google::protobuf::FieldDescriptorProto::TYPE_STRING);
  *desc.add_field() = field3;

  builder.addDescriptor(desc);

  google::protobuf::FieldDescriptorProto field4;
  field4.set_name("foobar");
  field4.set_number(4);
  field4.set_type_name("nested");
  field4.set_label(google::protobuf::FieldDescriptorProto::LABEL_REPEATED);
  builder.addField(field4);

  builder.finalize();

  std::string actual(builder.getProtoAsString());

  std::cout << actual << std::endl;

  auto msg = builder.createMessageInstance();

  const auto *descriptor = msg->GetDescriptor();
  // const auto* reflection = msg->GetReflection();

  const auto *field = descriptor->FindFieldByName("id");
  cr_assert_not_null(field);
  cr_assert_eq(field->type(), google::protobuf::FieldDescriptorProto::TYPE_INT32);

  const auto *descriptor2 = descriptor->FindNestedTypeByName("nested");
  cr_assert_not_null(descriptor2);

  const auto *field_foobar = descriptor->FindFieldByName("foobar");
  cr_assert_not_null(field_foobar);
  cr_assert_eq(field_foobar->type(), google::protobuf::FieldDescriptorProto::TYPE_MESSAGE);
  cr_assert(field_foobar->is_repeated());

  const google::protobuf::Descriptor *nestedType = field_foobar->message_type();
  cr_assert_not_null(nestedType);

  cr_assert_str_eq(nestedType->full_name().c_str(), "my.pkg.MyMessage.nested");
  cr_assert_eq(nestedType->field_count(), 2);

  const auto *nested_field1 = nestedType->FindFieldByName("foo");
  cr_assert_not_null(nested_field1);
  cr_assert_eq(nested_field1->type(), google::protobuf::FieldDescriptorProto::TYPE_STRING);

  const auto *nested_field2 = nestedType->FindFieldByName("bar");
  cr_assert_not_null(nested_field2);
  cr_assert_eq(nested_field2->type(), google::protobuf::FieldDescriptorProto::TYPE_STRING);
}

Test(schema, load_from_protobuf)
{
  const std::string proto = R"proto(
    syntax = "proto3";

    package my.pkg;

    message MyMessage {
      int32 id = 1;
      repeated nested foobar = 2;

      message nested {
        string foo = 1;
        string bar = 2;
      }
    }
    )proto";

  Schema builder(proto);

  const auto &messageProto = builder.getMessageProto();
  cr_assert(messageProto.name() == "MyMessage");
  cr_assert(messageProto.field_size() == 2);

  const auto &field1 = messageProto.field(0);
  cr_assert(field1.name() == "id");
  cr_assert(field1.number() == 1);
  cr_assert(field1.type() == google::protobuf::FieldDescriptorProto::TYPE_INT32);

  const auto &field2 = messageProto.field(1);
  cr_assert(field2.name() == "foobar");
  cr_assert(field2.label() == google::protobuf::FieldDescriptorProto::LABEL_REPEATED);
  cr_assert(field2.type_name() == "nested");

  // For nested types
  cr_assert(messageProto.nested_type_size() == 1);
  const auto &nested = messageProto.nested_type(0);
  cr_assert(nested.name() == "nested");
  cr_assert(nested.field_size() == 2);

}

Test(schema, extend_loaded_protobuf)
{
  const std::string proto = R"proto(
    syntax = "proto3";

    package my.pkg;

    message MyMessage {
      int32 id = 1;
      repeated nested foobar = 2;

      message nested {
        string foo = 1;
        string bar = 2;
      }
    }
    )proto";

  Schema builder(proto);

  google::protobuf::FieldDescriptorProto field1;
  field1.set_name("newfield");
  field1.set_number(builder.getMessageProto().field_size() + 1);
  field1.set_type(google::protobuf::FieldDescriptorProto::TYPE_FLOAT);
  builder.addField(field1);

  // std::cout << builder.getProtoAsString() << std::endl;

  builder.finalize();
  auto msg = builder.createMessageInstance();

  const auto *descriptor = msg->GetDescriptor();
  const auto *field = descriptor->FindFieldByName("newfield");
  cr_assert_not_null(field);
  cr_assert_eq(field->type(), google::protobuf::FieldDescriptorProto::TYPE_FLOAT);
  cr_assert_eq(field->number(), builder.getMessageProto().field_size());
}

void
setup(void)
{
}

void
teardown(void)
{
}

TestSuite(schema, .init = setup, .fini = teardown);
