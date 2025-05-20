add_unit_test(
  CRITERION
  TARGET test_schema
  SOURCES test_schema.cpp
  INCLUDES ${GOOGLEAPIS_PROTO_GRPC_SOURCES} ${PROJECT_SOURCE_DIR}/modules/grpc ${PROJECT_SOURCE_DIR}/modules/grpc/common
  DEPENDS schema-cpp)

# add_unit_test(LIBTEST CRITERION TARGET test_schema DEPENDS protobuf_filterx_message_cpp)
