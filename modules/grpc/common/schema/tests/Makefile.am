if ENABLE_GRPC

if ! OS_TYPE_MACOS
modules_grpc_common_schema_tests_TESTS = \
  modules/grpc/common/schema/tests/test_schema

check_PROGRAMS += ${modules_grpc_common_schema_tests_TESTS}
endif

modules_grpc_common_schema_tests_test_schema_SOURCES = \
  modules/grpc/common/schema/tests/test_schema.cpp

EXTRA_modules_grpc_common_schema_tests_test_schema_DEPENDENCIES = \
  $(top_builddir)/modules/grpc/protos/libgrpc-protos.la

modules_grpc_common_schema_tests_test_schema_CXXFLAGS = \
  $(TEST_CXXFLAGS) \
  $(PROTOBUF_CFLAGS) $(GRPCPP_CFLAGS) \
  -I$(top_srcdir)/modules/grpc/common/schema \
  -I$(top_builddir)/modules/grpc/common/schema

modules_grpc_common_schema_tests_test_schema_LDADD = \
  $(TEST_LDADD) \
  $(top_builddir)/modules/grpc/protos/libgrpc-protos.la

endif

EXTRA_DIST += \
    modules/grpc/common/schema/tests/CMakeLists.txt
