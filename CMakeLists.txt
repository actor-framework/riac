cmake_minimum_required(VERSION 2.8)
project(caf_riac C CXX)

# get header files; only needed by CMake generators,
# e.g., for creating proper Xcode projects
file(GLOB CAF_RIAC_HDRS "caf/riac/*.hpp")

# list cpp files excluding platform-dependent files
set (CAF_RIAC_SRCS
     src/announce_message_types.cpp
     src/nexus_proxy.cpp
     src/init_probe.cpp)

add_custom_target(libcaf_riac)

# build shared library if not compiling static only
if(NOT CAF_BUILD_STATIC_ONLY)
  add_library(libcaf_riac_shared SHARED ${CAF_RIAC_SRCS} ${CAF_RIAC_HDRS})
  target_link_libraries(libcaf_riac_shared ${LD_FLAGS} ${LIBCAF_LIBRARIES})
  set(LIBRARY_SOVERSION ${CAF_VERSION_MAJOR})
  set_target_properties(libcaf_riac_shared
                        PROPERTIES
                        SOVERSION ${LIBRARY_SOVERSION}
                        VERSION ${CAF_VERSION}
                        OUTPUT_NAME caf_riac)
  if(NOT WIN32)
    install(TARGETS libcaf_riac_shared LIBRARY DESTINATION lib)
  endif()
  add_dependencies(libcaf_riac_shared libcaf_riac)
endif()

# build static library if --build-static or --build-static-only was set
if(CAF_BUILD_STATIC_ONLY OR CAF_BUILD_STATIC)
  add_library(libcaf_riac_static STATIC ${CAF_RIAC_HDRS} ${CAF_RIAC_SRCS})
  set_target_properties(libcaf_riac_static PROPERTIES OUTPUT_NAME caf_riac_static)
  install(TARGETS libcaf_riac_static ARCHIVE DESTINATION lib)
  add_dependencies(libcaf_riac_static libcaf_riac)
endif()

link_directories(${LD_DIRS})
include_directories(. ${INCLUDE_DIRS})

# install includes
install(DIRECTORY caf/ DESTINATION include/caf FILES_MATCHING PATTERN "*.hpp")
