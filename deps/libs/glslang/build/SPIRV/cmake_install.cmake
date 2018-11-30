# Install script for directory: C:/VulkanSDK/1.1.85.0/glslang/SPIRV

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
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/VulkanSDK/1.1.85.0/glslang/build/SPIRV/Debug/SPVRemapperd.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/VulkanSDK/1.1.85.0/glslang/build/SPIRV/Release/SPVRemapper.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/VulkanSDK/1.1.85.0/glslang/build/SPIRV/MinSizeRel/SPVRemapper.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/VulkanSDK/1.1.85.0/glslang/build/SPIRV/RelWithDebInfo/SPVRemapper.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/VulkanSDK/1.1.85.0/glslang/build/SPIRV/Debug/SPIRVd.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/VulkanSDK/1.1.85.0/glslang/build/SPIRV/Release/SPIRV.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/VulkanSDK/1.1.85.0/glslang/build/SPIRV/MinSizeRel/SPIRV.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/VulkanSDK/1.1.85.0/glslang/build/SPIRV/RelWithDebInfo/SPIRV.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/SPIRV" TYPE FILE FILES
    "C:/VulkanSDK/1.1.85.0/glslang/SPIRV/bitutils.h"
    "C:/VulkanSDK/1.1.85.0/glslang/SPIRV/spirv.hpp"
    "C:/VulkanSDK/1.1.85.0/glslang/SPIRV/GLSL.std.450.h"
    "C:/VulkanSDK/1.1.85.0/glslang/SPIRV/GLSL.ext.EXT.h"
    "C:/VulkanSDK/1.1.85.0/glslang/SPIRV/GLSL.ext.KHR.h"
    "C:/VulkanSDK/1.1.85.0/glslang/SPIRV/GlslangToSpv.h"
    "C:/VulkanSDK/1.1.85.0/glslang/SPIRV/hex_float.h"
    "C:/VulkanSDK/1.1.85.0/glslang/SPIRV/Logger.h"
    "C:/VulkanSDK/1.1.85.0/glslang/SPIRV/SpvBuilder.h"
    "C:/VulkanSDK/1.1.85.0/glslang/SPIRV/spvIR.h"
    "C:/VulkanSDK/1.1.85.0/glslang/SPIRV/doc.h"
    "C:/VulkanSDK/1.1.85.0/glslang/SPIRV/SpvTools.h"
    "C:/VulkanSDK/1.1.85.0/glslang/SPIRV/disassemble.h"
    "C:/VulkanSDK/1.1.85.0/glslang/SPIRV/GLSL.ext.AMD.h"
    "C:/VulkanSDK/1.1.85.0/glslang/SPIRV/GLSL.ext.NV.h"
    "C:/VulkanSDK/1.1.85.0/glslang/SPIRV/SPVRemapper.h"
    "C:/VulkanSDK/1.1.85.0/glslang/SPIRV/doc.h"
    )
endif()

