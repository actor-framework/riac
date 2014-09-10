cmake_minimum_required(VERSION 2.8)
project(caf_riac C CXX)

# get header files; only needed by CMake generators,
# e.g., for creating proper Xcode projects
file(GLOB RIAC_HDRS "caf/riac/*.hpp")

# list cpp files excluding platform-dependent files
set (RIAC_SRCS
     src/init.cpp
     src/interfaces.cpp)

# build shared library if not compiling static only
if(NOT CAF_BUILD_STATIC_ONLY)
  add_library(libcafriac SHARED ${RIAC_SRCS} ${RIAC_HDRS})
  target_link_libraries(libcafriac ${LD_FLAGS} ${LIBCAF_LIBRARIES})
  set(LIBRARY_SOVERSION ${CAF_VERSION_MAJOR})
  set_target_properties(libcafriac
                        PROPERTIES
                        SOVERSION ${LIBRARY_SOVERSION}
                        VERSION ${CAF_VERSION}
                        OUTPUT_NAME cafriac)
  if(NOT MINGW)
    install(TARGETS libcafriac LIBRARY DESTINATION lib)
  endif()
endif ()

# build static library only if --build-static or --build-static-only was set
if(CAF_BUILD_STATIC_ONLY OR CAF_BUILD_STATIC)
  add_library(libcafriacStatic STATIC ${RIAC_HDRS} ${RIAC_SRCS})
  set_target_properties(libcafriacStatic PROPERTIES OUTPUT_NAME cafriac_static)
  install(TARGETS libcafriacStatic ARCHIVE DESTINATION lib)
endif ()

link_directories(${LD_DIRS})
include_directories(. ${INCLUDE_DIRS})

# install includes
install(DIRECTORY caf/ DESTINATION include/caf FILES_MATCHING PATTERN "*.hpp")
