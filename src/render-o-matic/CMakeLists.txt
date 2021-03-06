# Project configuration
cmake_minimum_required(VERSION 2.8)
project(render-o-matic)
set(EXECUTABLE_OUTPUT_PATH ../bin)

# Include directories
include_directories(../../include/libbvh)
file(GLOB LIBBVH_HDR "../../include/libbvh/*.h")
include_directories(../../include/libaux)
file(GLOB LIBAUX_HDR "../../include/libaux/*.h")

if (CUDA_FOUND)
  include_directories(../../include/libbvhcuda)
  file(GLOB LIBBVHCUDA_HDR "../../include/libcuda/*.h")
else (CUDA_FOUND)
  set(LIBBVHCUDA_HDR "")
endif (CUDA_FOUND)

# Add executable
add_executable(render-o-matic 
  main.cpp
  ${LIBBVH_HDR}
  ${LIBAUX_HDR}
  ${LIBBVHCUDA_HDR})

message("IL libraries: ${IL_LIBRARY}")

# Link with libraries
target_link_libraries(render-o-matic
  bvh
  aux
  ${SDL_LIBRARY}
  ${IL_LIBRARIES}
  ${GSL_LIBRARIES})

if (CUDA_FOUND)
  target_link_libraries(render-o-matic bvhcuda)
endif (CUDA_FOUND)

