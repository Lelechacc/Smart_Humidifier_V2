# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/frameworks/esp-idf-v5.3.1/components/bootloader/subproject"
  "E:/V9/Humidifier_V9_Core/build/bootloader"
  "E:/V9/Humidifier_V9_Core/build/bootloader-prefix"
  "E:/V9/Humidifier_V9_Core/build/bootloader-prefix/tmp"
  "E:/V9/Humidifier_V9_Core/build/bootloader-prefix/src/bootloader-stamp"
  "E:/V9/Humidifier_V9_Core/build/bootloader-prefix/src"
  "E:/V9/Humidifier_V9_Core/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "E:/V9/Humidifier_V9_Core/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "E:/V9/Humidifier_V9_Core/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
