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

#ifndef OBJECT_PROTOBUFMESSAGE_MESSAGE_HPP
#define OBJECT_PROTOBUFMESSAGE_MESSAGE_HPP

#include "syslog-ng.h"
#include "schema/grpc-schema.hpp"

#include "compat/cpp-start.h"
#include "filterx/object-dict-interface.h"
#include "protobuf-message.h"
#include "compat/cpp-end.h"


#include "google/protobuf/message.h"

#include <string>
#include <unordered_map>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/io/zero_copy_stream.h>

typedef struct FilterXProtobufMessage_ FilterXProtobufMessage;

namespace syslogng {
namespace grpc {
namespace clickhouse {
namespace filterx {

class InMemorySourceTree : public google::protobuf::compiler::SourceTree {
  public:
      void AddFile(const std::string& filename, const std::string& content) {
          files_[filename] = content;
      }

      google::protobuf::io::ZeroCopyInputStream* Open(const std::string& filename) override {
          if (files_.count(filename)) {
              return new google::protobuf::io::ArrayInputStream(files_[filename].data(), files_[filename].size());
          }
          return nullptr;
      }

      std::string GetLastErrorMessage() const { return last_error_; }

  private:
      std::unordered_map<std::string, std::string> files_;
      std::string last_error_;
  };

  class SimpleErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector {
  public:
      void AddError(const std::string& filename, int line, int column, const std::string& message) override {
          std::cerr << "Error in " << filename << ":" << line << ":" << column << ": " << message << std::endl;
      }
  };

class DynamicProtoLoader
{
public:
  // Constructor from .proto file path or content
  DynamicProtoLoader(FilterXProtobufMessage *super_, const std::string& proto_content, const std::string& proto_filename = "dynamic.proto");

  // Constructor from schema-like map (name -> type); creates .proto dynamically
  DynamicProtoLoader(FilterXProtobufMessage *super_, const std::string& message_name, const std::map<std::string, std::string>& schema_map);

  // Instantiate a new message from loaded schema
  std::unique_ptr<google::protobuf::Message> CreateMessageInstance() const;

  // Get message descriptor
  const google::protobuf::Descriptor* GetDescriptor() const;

private:
  std::unique_ptr<google::protobuf::compiler::Importer> importer_;
  std::unique_ptr<google::protobuf::DynamicMessageFactory> message_factory_;
  const google::protobuf::Descriptor* descriptor_ = nullptr;
  std::string message_type_;
  FilterXProtobufMessage *super;

  void LoadProtoFromString(const std::string& proto_content, const std::string& proto_filename);
  std::string GenerateProtoFromMap(const std::string& message_name, const std::map<std::string, std::string>& schema_map);
};

}
}
}
}

struct FilterXProtobufMessage_
{
  FilterXFunction super;
  FilterXExpr *input;
  syslogng::grpc::clickhouse::filterx::DynamicProtoLoader *cpp;
};

#endif
