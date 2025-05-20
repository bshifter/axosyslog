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

#ifndef PROTOBUF_SCHEMA_HPP
#define PROTOBUF_SCHEMA_HPP

#include "syslog-ng.h"

#include "compat/cpp-start.h"
#include "filterx/object-dict-interface.h"
#include "protobuf-message.h"
#include "compat/cpp-end.h"


#include "google/protobuf/message.h"

#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>

#include <string>
#include <vector>
#include <variant>
#include <optional>

namespace syslogng {
namespace grpc {
namespace common {

// class DynField {
//     public:
//         DynField(
//             std::string name,
//             int number,
//             google::protobuf::FieldDescriptorProto::Type type,
//             google::protobuf::FieldDescriptorProto::Label label =
//                 google::protobuf::FieldDescriptorProto::LABEL_OPTIONAL);

//         // For nested message type
//         void setNestedMessage(
//             std::string nestedMessageName,
//             std::vector<DynField> nestedFields);

//         google::protobuf::FieldDescriptorProto toProto() const;

//         static bool setField(google::protobuf::Message* msg, const std::string& fieldName, const std::variant<int32_t, std::string, std::vector<google::protobuf::Message*>>& value);
//         static std::variant<int32_t, std::string, std::vector<google::protobuf::Message*>> getField(const google::protobuf::Message& msg, const std::string& fieldName);

//     private:
//         std::string name_;
//         int number_;
//         google::protobuf::FieldDescriptorProto::Type type_;
//         google::protobuf::FieldDescriptorProto::Label label_;

//         bool isNested_ = false;
//         std::string nestedMessageName_;
//         std::vector<DynField> nestedFields_;
//     };

class ProtoErrorCollector : public google::protobuf::io::ErrorCollector {
    public:
        void AddError(int line, int column, const std::string& message) override {
            errors_.emplace_back("Error at line " + std::to_string(line) +
                                 ", col " + std::to_string(column) +
                                 ": " + message);
        }

        void AddWarning(int line, int column, const std::string& message) override {
            warnings_.emplace_back("Warning at line " + std::to_string(line) +
                                   ", col " + std::to_string(column) +
                                   ": " + message);
        }

        const std::vector<std::string>& errors() const { return errors_; }
        const std::vector<std::string>& warnings() const { return warnings_; }

    private:
        std::vector<std::string> errors_;
        std::vector<std::string> warnings_;
};

class Schema {
public:
    Schema(const std::string& packageName, const std::string& messageName);
    Schema(const google::protobuf::FileDescriptorProto& proto, google::protobuf::DescriptorProto* msg);
    Schema(const std::string& protoText, google::protobuf::io::ErrorCollector* errorCollector = nullptr);
    void addField(const google::protobuf::FieldDescriptorProto& field);
    void addDescriptor(const google::protobuf::DescriptorProto& desc);
    std::string getProtoAsString() const;
    const google::protobuf::FileDescriptorProto& getFileProto() const;
    google::protobuf::DescriptorProto& getMessageProto() const;
    void finalize();
    std::unique_ptr<google::protobuf::Message> createMessageInstance() const;
    bool loadFromProtoString(const std::string& protoText);
private:
    google::protobuf::FileDescriptorProto fileProto_;
    google::protobuf::DescriptorProto* messageProto_;

    std::unique_ptr<google::protobuf::DescriptorPool> descriptorPool_;
    std::unique_ptr<google::protobuf::DynamicMessageFactory> messageFactory_;
    const google::protobuf::Descriptor* messageDescriptor_ = nullptr;
    const google::protobuf::Message* messagePrototype_ = nullptr;
};

}
}
}

#endif
