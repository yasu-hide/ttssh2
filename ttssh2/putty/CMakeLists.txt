﻿project(libputty)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/")

if(MINGW)
  add_definitions("-D_WINDOWS")
endif()

set(PUTTY_SRC
  ../../libs/putty/putty.h
  ../../libs/putty/pageant.h
  ../../libs/putty/conf.c
  ../../libs/putty/misc.h
  ../../libs/putty/misc.c
  ../../libs/putty/tree234.h
  ../../libs/putty/tree234.c
  ../../libs/putty/version.h
  ../../libs/putty/version.c
  ../../libs/putty/windows/winmisc.c
  ../../libs/putty/windows/winpgntc.c
  ../../libs/putty/windows/winsecur.h
  ../../libs/putty/windows/winsecur.c
  )

source_group(
  "putty"
  FILES
  ${PUTTY_SRC}
  )

set(SRC
  libputty.c
  libputty.h
  )

include_directories(
  ../../libs/putty
  ../../libs/putty/windows
  .
  )

add_library(
  libputty
  ${SRC}
  ${PUTTY_SRC}
  )
