# Project configuration
cmake_minimum_required(VERSION 2.8)
project(aux)

# Include directories
include_directories(../../include/libaux)
file(GLOB LIBAUX_HDR "../../include/libaux/*.h")

# Add library
add_library(aux
  image.cpp
  indexer.cpp
  mathaux.cpp
  nmlparser.cpp
  params.cpp
  path.cpp
  pkt.cpp
  utils.cpp
  ${LIBAUX_HDR})

# Link with libraries


