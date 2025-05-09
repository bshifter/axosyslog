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

typedef struct FilterXProtobufMessage_ FilterXProtobufMessage;

// FilterXObject *_filterx_protobuf_message_clone(FilterXObject *s);

namespace syslogng {
namespace grpc {
namespace clickhouse {
namespace filterx {

class Message
{
public:
  // Message(FilterXProtobufMessage *super);
  // Message(FilterXProtobufMessage *super, FilterXObject *protobuf_object);
  Message(FilterXProtobufMessage *super, Schema *schema);
  Message(Message &o) = delete;
  Message(Message &&o) = delete;
  // std::string marshal(void);
  // const google::protobuf::Message &get_value() const;
  // std::string repr() const;

  // bool set_attribute(const std::string &key, const std::string &value);
  // bool set_data(const std::string &data);
private:
  FilterXProtobufMessage *super;
  Schema *schema;
  Message(const Message &o, FilterXProtobufMessage *super);
  // friend FilterXObject *::_filterx_protobuf_message_clone(FilterXObject *s);
};

}
}
}
}

struct FilterXProtobufMessage_
{
  FilterXFunction super;
  syslogng::grpc::clickhouse::filterx::Message *cpp;
};

#endif
