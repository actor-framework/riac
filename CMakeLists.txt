cmake_minimum_required(VERSION 2.8)
project(caf_probe_event C CXX)

# get header files; only needed by CMake generators,
# e.g., for creating proper Xcode projects
file(GLOB PROBEi_EVENT_HDRS "caf/probe-event/*.hpp")

# list cpp files excluding platform-dependent files
set (PROBE_EVENT_SRCS )

link_directories(${LD_DIRS})
include_directories(. ${INCLUDE_DIRS})

# install includes
install(DIRECTORY caf/ DESTINATION include/caf FILES_MATCHING PATTERN "*.hpp")
