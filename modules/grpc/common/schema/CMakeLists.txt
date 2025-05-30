# set(
#     ${PROJECT_SOURCE_DIR}/modules/grpc/common/schema/schema.hpp
#     ${PROJECT_SOURCE_DIR}/modules/grpc/common/schema/schema.cpp
#     ${PROJECT_SOURCE_DIR}/modules/grpc/common/schema/grpc-schema.hpp
#     ${PROJECT_SOURCE_DIR}/modules/grpc/common/schema/grpc-schema.cpp
#     PARENT_SCOPE)


set(SCHEMA_CPP_SOURCES
schema.hpp
schema.cpp
grpc-schema.hpp
grpc-schema.cpp
)

add_module(
    TARGET schema-cpp
    SOURCES ${SCHEMA_CPP_SOURCES}
    DEPENDS ${MODULE_GRPC_LIBS} grpc-protos grpc-common-cpp
    INCLUDES ${GOOGLEAPIS_PROTO_GRPC_SOURCES} ${PROJECT_SOURCE_DIR}/modules/grpc ${PROJECT_SOURCE_DIR}/modules/grpc/common
    LIBRARY_TYPE STATIC
    )

set_target_properties(schema-cpp PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations")

add_test_subdirectory(tests)
