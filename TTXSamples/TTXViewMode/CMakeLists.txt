﻿project(TTXViewMode)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/")

set(COMMON_SRC
  ../../teraterm/common/dlglib.h
  ../../teraterm/common/dlglib.c
  ../../teraterm/common/dlglib_cpp.cpp
  ../../teraterm/common/dlglib_tmpl.cpp
  ../../teraterm/common/ttlib.c
  )

source_group(
  "common"
  FILES
  ${COMMON_SRC}
  )

set(SRC
  resource.h
  TTXViewMode.c
  TTXViewMode.h
  TTXViewMode.rc
  ReadMe.txt
  ReadMe-ja.txt
  )

include_directories(
  ../../teraterm/teraterm
  ../../teraterm/common
  )

add_library(
  TTXViewMode SHARED
  ${SRC}
  ${COMMON_SRC}
  )

set_target_properties(
  TTXViewMode
  PROPERTIES
  PREFIX ""
  )

target_link_libraries(
  TTXViewMode
  ttpcmn
  #
  gdi32
  )
