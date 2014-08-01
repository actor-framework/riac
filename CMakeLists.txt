cmake_minimum_required(VERSION 2.8)
project(caf_probe C CXX)

# get header files; only needed by CMake generators,
# e.g., for creating proper Xcode projects
file(GLOB PROBE_HDRS "caf/probe/*.hpp")

# list cpp files excluding platform-dependent files
set (PROBE_SRCS
     src/init.cpp)

# build shared library if not compiling static only
if(NOT CAF_BUILD_STATIC_ONLY)
  add_library(libcaf_probe SHARED ${PROBE_SRCS} ${PROBE_HDRS})
  target_link_libraries(libcaf_probe ${LD_FLAGS} ${LIBCAF_LIBRARIES})
  set(LIBRARY_SOVERSION ${CAF_VERSION_MAJOR})
  set_target_properties(libcaf_probe
                        PROPERTIES
                        SOVERSION ${LIBRARY_SOVERSION}
                        VERSION ${CAF_VERSION}
                        OUTPUT_NAME caf_probe)
  if(NOT MINGW)
    install(TARGETS libcaf_probe LIBRARY DESTINATION lib)
  endif()
endif ()

# build static library only if --build-static or --build-static-only was set
if(CAF_BUILD_STATIC_ONLY OR CAF_BUILD_STATIC)
  add_library(libcaf_probeStatic STATIC ${PROBE_HDRS} ${PROBE_SRCS})
  set_target_properties(libcaf_probeStatic PROPERTIES OUTPUT_NAME caf_probe_static)
  install(TARGETS libcaf_probeStatic ARCHIVE DESTINATION lib)
endif ()

link_directories(${LD_DIRS})
include_directories(. ${INCLUDE_DIRS})

# install includes
install(DIRECTORY caf/ DESTINATION include/caf FILES_MATCHING PATTERN "*.hpp")
