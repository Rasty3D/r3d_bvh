# Project configuration
cmake_minimum_required(VERSION 2.8)
project(bvhcuda)

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

# Add library
cuda_add_library(bvhcuda
  bvhcuda.cu
  bvhgpu_5.cu
  ${LIBAUX_HDR}
  ${LIBBVH_HDR}
  ${LIBBVHCUDA_HDR})

