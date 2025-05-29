if ENABLE_GRPC

noinst_LTLIBRARIES += modules/grpc/common/filterx/libgrpc-filterx_cpp.la

modules_grpc_common_filterx_libgrpc_filterx_cpp_la_SOURCES = \
  modules/grpc/common/filterx/protobuf-message/protobuf-field.hpp \
  modules/grpc/common/filterx/protobuf-message/protobuf-field.cpp \
  modules/grpc/common/filterx/protobuf-message/protobuf-message.h \
  modules/grpc/common/filterx/protobuf-message/protobuf-message.hpp \
  modules/grpc/common/filterx/protobuf-message/protobuf-message.cpp

modules_grpc_common_filterx_libgrpc_filterx_cpp_la_CXXFLAGS = \
  $(AM_CXXFLAGS) \
  $(PROTOBUF_CFLAGS) \
  $(GRPCPP_CFLAGS) \
  $(GRPC_COMMON_CFLAGS) \
  -I$(top_srcdir)/modules/grpc \
  -I$(top_srcdir)/modules/grpc/commonn/filterx \
  -I$(top_builddir)/modules/grpc/common/filterx

modules_grpc_common_filterx_libgrpc_filterx_cpp_la_LIBADD = $(MODULE_DEPS_LIBS) $(PROTOBUF_LIBS) $(GRPCPP_LIBS)
modules_grpc_common_filterx_libgrpc_filterx_cpp_la_LDFLAGS = $(MODULE_LDFLAGS)
EXTRA_modules_grpc_common_filterx_libgrpc_filterx_cpp_la_DEPENDENCIES = $(MODULE_DEPS_LIBS)

module_LTLIBRARIES += modules/grpc/common/filterx/libgrpc-filterx.la

modules_grpc_common_filterx_libgrpc_filterx_la_SOURCES = \
  modules/grpc/common/filterx/grpc-filterx.c

modules_grpc_common_filterx_libgrpc_filterx_la_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  $(GRPC_COMMON_CFLAGS) \
  -I$(top_srcdir)/modules/grpc/common/filterx \
  -I$(top_builddir)/modules/grpc/common/filterx \
  -I$(top_srcdir)/modules/grpc

modules_grpc_common_filterx_libgrpc_filterx_la_LIBADD = \
  $(MODULE_DEPS_LIBS) \
  $(GRPC_COMMON_LIBS) \
  $(top_builddir)/modules/grpc/common/filterx/libgrpc-filterx_cpp.la

nodist_EXTRA_modules_grpc_common_filterx_libgrpc_filterx_la_SOURCES = force-cpp-linker-with-default-stdlib.cpp

modules_grpc_common_filterx_libgrpc_filterx_la_LDFLAGS = $(MODULE_LDFLAGS)
EXTRA_modules_grpc_common_filterx_libgrpc_filterx_la_DEPENDENCIES = \
  $(MODULE_DEPS_LIBS) \
  $(GRPC_COMMON_LIBS) \
  $(top_builddir)/modules/grpc/common/filterx/libgrpc-filterx_cpp.la

endif

EXTRA_DIST += \
  modules/grpc/common/filterx/CMakeLists.txt

.PHONY: modules/grpc/common/filterx/ mod-grpc-filterx
