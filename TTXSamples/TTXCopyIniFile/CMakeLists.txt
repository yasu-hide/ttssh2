﻿project(TTXCopyIniFile)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/")

set(SRC
  TTXCopyIniFile.c
  ReadMe.txt
  ReadMe-ja.txt
  )

include_directories(
  ../../teraterm/teraterm
  ../../teraterm/common
  )

add_library(
  TTXCopyIniFile SHARED
  ${SRC}
  )

set_target_properties(
  TTXCopyIniFile
  PROPERTIES
  PREFIX ""
  )
