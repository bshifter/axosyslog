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

#include "compat/cpp-start.h"
#include "filterx/object-dict-interface.h"
#include "protobuf-message.h"
#include "compat/cpp-end.h"
#include "schema.hpp"

#include "google/protobuf/message.h"

#include <string>
#include <unordered_map>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/io/zero_copy_stream.h>

typedef struct FilterXProtobufMessage_ FilterXProtobufMessage;

namespace syslogng {
namespace grpc {
namespace common {
namespace filterx {
class DynamicProtoLoader
{
public:
  DynamicProtoLoader(FilterXProtobufMessage *super_, const std::string& proto_content, const std::string& proto_filename = "dynamic.proto");
  DynamicProtoLoader(FilterXProtobufMessage *super_, const std::string& message_name, const std::map<std::string, std::string>& schema_map);
  Schema& getSchema();
private:
  FilterXProtobufMessage *super;
  Schema _schema;
};

}
}
}
}

struct FilterXProtobufMessage_
{
  FilterXFunction super;
  FilterXExpr *input;
  syslogng::grpc::common::filterx::DynamicProtoLoader *cpp;
};

#endif
