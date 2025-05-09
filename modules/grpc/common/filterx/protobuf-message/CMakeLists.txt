if(NOT ENABLE_CPP)
  return()
endif()

# C++ code

set(PROTOBUF_MESSAGE_CPP_SOURCES
  object-protobuf.h
  object-protobuf-message.cpp
  object-protobuf-message.hpp
)

add_module(
  TARGET protobuf_filterx_message_cpp
  SOURCES ${PROTOBUF_MESSAGE_CPP_SOURCES}
  DEPENDS ${MODULE_GRPC_LIBS} grpc-protos grpc-common-cpp
  INCLUDES ${GOOGLEAPIS_PROTO_GRPC_SOURCES} ${PROJECT_SOURCE_DIR}/modules/grpc ${PROJECT_SOURCE_DIR}/modules/grpc/common
  LIBRARY_TYPE STATIC
)

set_target_properties(protobuf_filterx_message_cpp PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations")
