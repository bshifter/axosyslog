set(GRPC_SCHEMA_SOURCES
    ${PROJECT_SOURCE_DIR}/modules/grpc/common/schema/schema.hpp
    ${PROJECT_SOURCE_DIR}/modules/grpc/common/schema/schema.cpp
    ${PROJECT_SOURCE_DIR}/modules/grpc/common/schema/grpc-schema.hpp
    ${PROJECT_SOURCE_DIR}/modules/grpc/common/schema/grpc-schema.cpp
    PARENT_SCOPE)

add_test_subdirectory(tests)
