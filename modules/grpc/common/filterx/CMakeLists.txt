if(NOT ENABLE_GRPC)
  return()
endif()

set(GRPC_COMMON_PLUGINS_SOURCES
  grpc-filterx.c
)

add_module(
  TARGET grpc-filterx
  DEPENDS grpc-common-cpp
  INCLUDES ${PROJECT_SOURCE_DIR}/modules/grpc
  SOURCES ${GRPC_COMMON_PLUGINS_SOURCES}
)

set_target_properties(grpc-filterx PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${CMAKE_INSTALL_PREFIX}/lib/syslog-ng")

add_subdirectory(protobuf-message)
target_link_libraries(grpc-filterx PRIVATE protobuf_filterx_message_cpp)
