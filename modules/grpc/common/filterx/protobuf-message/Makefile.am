if ENABLE_GRPC

# noinst: Built as part of a larger libcommon module as static library
noinst_LTLIBRARIES += modules/grpc/common/filterx/libfilterx.la

modules_grpc_common_filterx_libfilterx_la_SOURCES = \
  modules/grpc/common/filterx/protobuf-message.h \
  modules/grpc/common/filterx/protobuf-message.cpp \
  modules/grpc/common/filterx/protobuf-message.hpp

modules_grpc_common_filterx_libfilterx_la_CXXFLAGS = \
  $(AM_CXXFLAGS) \
  $(PROTOBUF_CFLAGS) \
  $(GRPCPP_CFLAGS) \
  -I$(GOOGLEAPIS_PROTO_BUILDDIR) \
  -I$(top_srcdir)/modules/grpc/common/filterx \
  -I$(top_builddir)/modules/grpc/common/filterx \
  -Wno-deprecated-declarations

modules_grpc_common_filterx_libfilterx_la_LIBADD = \
  $(MODULE_DEPS_LIBS) \
  $(PROTOBUF_LIBS) \
  $(top_builddir)/modules/grpc/protos/libgrpc-protos.la

modules_grpc_common_filterx_libfilterx_la_LDFLAGS = $(MODULE_LDFLAGS)

EXTRA_modules_grpc_common_filterx_libfilterx_la_DEPENDENCIES = \
  $(MODULE_DEPS_LIBS) \
  $(top_builddir)/modules/grpc/protos/libgrpc-protos.la

endif

EXTRA_DIST += \
  modules/grpc/common/filterx/CMakeLists.txt
