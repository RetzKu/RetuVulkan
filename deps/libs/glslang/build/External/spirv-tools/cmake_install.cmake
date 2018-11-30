# Install script for directory: C:/VulkanSDK/1.1.85.0/glslang/External/spirv-tools

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/glslang")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/spirv-tools" TYPE FILE FILES
    "C:/VulkanSDK/1.1.85.0/glslang/External/spirv-tools/include/spirv-tools/libspirv.h"
    "C:/VulkanSDK/1.1.85.0/glslang/External/spirv-tools/include/spirv-tools/libspirv.hpp"
    "C:/VulkanSDK/1.1.85.0/glslang/External/spirv-tools/include/spirv-tools/optimizer.hpp"
    "C:/VulkanSDK/1.1.85.0/glslang/External/spirv-tools/include/spirv-tools/linker.hpp"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES
    "C:/VulkanSDK/1.1.85.0/glslang/build/External/spirv-tools/SPIRV-Tools.pc"
    "C:/VulkanSDK/1.1.85.0/glslang/build/External/spirv-tools/SPIRV-Tools-shared.pc"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/VulkanSDK/1.1.85.0/glslang/build/External/spirv-tools/external/cmake_install.cmake")
  include("C:/VulkanSDK/1.1.85.0/glslang/build/External/spirv-tools/source/cmake_install.cmake")
  include("C:/VulkanSDK/1.1.85.0/glslang/build/External/spirv-tools/tools/cmake_install.cmake")
  include("C:/VulkanSDK/1.1.85.0/glslang/build/External/spirv-tools/test/cmake_install.cmake")
  include("C:/VulkanSDK/1.1.85.0/glslang/build/External/spirv-tools/examples/cmake_install.cmake")

endif()

