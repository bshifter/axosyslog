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

#include "schema.hpp"

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

#include <unistd.h>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <optional>

using namespace syslogng::grpc::common;
using namespace google::protobuf;
using namespace google::protobuf::compiler;

Schema::Schema(const std::string& packageName, const std::string& messageName)
{
    fileProto_.set_name("generated.proto");
    fileProto_.set_package(packageName);

    DescriptorProto* msg = fileProto_.add_message_type();
    msg->set_name(messageName);
    messageProto_ = msg;
}

Schema::Schema(const google::protobuf::FileDescriptorProto& proto,
  google::protobuf::DescriptorProto* msg)
: fileProto_(proto), messageProto_(nullptr)
{
  // Find the corresponding message by name in the copied fileProto_
  for (int i = 0; i < fileProto_.message_type_size(); ++i) {
    if (fileProto_.message_type(i).name() == msg->name()) {
      messageProto_ = fileProto_.mutable_message_type(i);
      break;
    }
  }

  if (!messageProto_) {
    throw std::runtime_error("Message not found in copied descriptor");
  }
}

Schema::Schema(const std::string& protoText, google::protobuf::io::ErrorCollector* errorCollector)
{
    using namespace google::protobuf;

    io::ArrayInputStream input(protoText.data(), protoText.size());
    io::Tokenizer tokenizer(&input, errorCollector);
    compiler::Parser parser;

    fileProto_.set_name("generated.proto");
    if (!parser.Parse(&tokenizer, &fileProto_)) {
        throw std::runtime_error("Failed to parse proto file content");
    }

    if (fileProto_.message_type_size() == 0) {
        throw std::runtime_error("Empty proto file content");
    }

    DescriptorProto* messageProto = fileProto_.mutable_message_type(0);
    messageProto_ = messageProto;
}

void
Schema::addField(const FieldDescriptorProto& field)
{
    *(messageProto_->add_field()) = field;
}

void
Schema::addDescriptor(const google::protobuf::DescriptorProto& desc)
{
  *(messageProto_->add_nested_type()) = desc;
}

std::string
Schema::getProtoAsString() const
{
    return fileProto_.DebugString();
}

const FileDescriptorProto&
Schema::getFileProto() const
{
    return fileProto_;
}

DescriptorProto&
Schema::getMessageProto() const
{
    return *messageProto_;
}

void
Schema::finalize()
{
  if (messagePrototype_) return;  // Already finalized

  descriptorPool_ = std::make_unique<google::protobuf::DescriptorPool>();
  messageFactory_ = std::make_unique<google::protobuf::DynamicMessageFactory>();

  const auto* fileDescriptor = descriptorPool_->BuildFile(fileProto_);
  if (!fileDescriptor) {
      throw std::runtime_error("Failed to build FileDescriptor in finalize().");
  }

  messageDescriptor_ = fileDescriptor->FindMessageTypeByName(messageProto_->name());
  if (!messageDescriptor_) {
      throw std::runtime_error("Message type not found in FileDescriptor in finalize().");
  }

  messagePrototype_ = messageFactory_->GetPrototype(messageDescriptor_);
  if (!messagePrototype_) {
      throw std::runtime_error("Failed to get message prototype in finalize().");
  }
}

std::unique_ptr<google::protobuf::Message>
Schema::createMessageInstance() const
{
  if (!messagePrototype_) {
      throw std::runtime_error("Schema::finalize() must be called before createMessageInstance().");
  }
  return std::unique_ptr<google::protobuf::Message>(messagePrototype_->New());
}
