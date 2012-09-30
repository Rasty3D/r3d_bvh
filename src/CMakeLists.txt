# Project configuration
cmake_minimum_required(VERSION 2.8)
project(bvh)
set(CMAKE_MODULE_PATH ../cmake)

# Compiler options
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -O3")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wall -O3")

# Find libraries
message("Looking for libraries...")

# Set CUDA
find_package(CUDA)

# Set Doxygen
set(DOXYFILE_EXTRA_SOURCES "../include")
set(DOXYFILE_OUTPUT_DIR "../docs")
set(DOXYFILE_LATEX "ON")
include(UseDoxygen OPTIONAL)

if (DOXYGEN_FOUND)
  message("DOXYGEN found")
else (DOXYGEN_FOUND)
  message("DOXYGEN not found")
endif (DOXYGEN_FOUND)

# Set SDL
find_package(SDL REQUIRED)

if (SDL_FOUND)
  message("SDL found")
else (SDL_FOUND)
  message("SDL not found")
endif (SDL_FOUND)

# Set DevIL
find_package(DevIL REQUIRED)

if (IL_FOUND)
  message("DevIL found")
else (IL_FOUND)
  message("DevIL not found")
endif (IL_FOUND)

# Set GSL
find_package(GSL REQUIRED)

if (GSL_FOUND)
  message("GSL found")
else (GSL_FOUND)
  message("GSL not found")
endif (GSL_FOUND)

# Set subdirectories
if (CUDA_FOUND)
  message("CUDA found")
  add_definitions(-DCUDA)
  add_subdirectory(libbvhcuda)
  set(DOXYFILE_PREDEFINED "CUDA")
else (CUDA_FOUND)
  message("CUDA not found")
endif (CUDA_FOUND)

add_subdirectory(libaux)
add_subdirectory(libbvh)
add_subdirectory(bvhc)
add_subdirectory(render-o-matic)

