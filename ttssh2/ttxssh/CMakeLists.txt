﻿project(ttxssh)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/")

include(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/lib_zlib.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/lib_openssl.cmake)

if (MSVC)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DELAYLOAD:user32.dll /DELAYLOAD:dnsapi.dll")
endif()

add_library(
  ttxssh SHARED
  )

target_sources(
  ttxssh
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}/puttyversion.h
  ../matcher/matcher.c
  arc4random.c
  arc4random.h
  auth.c
  auth.h
  buffer.c
  buffer.h
  chacha.c
  chacha.h
  cipher.h
  cipher-ctr.c
  config.h
  crypt.c
  crypt.h
  dns.c
  dns.h
  ed25519.c
  ed25519_bcrypt_pbkdf.c
  ed25519_blf.h
  ed25519_blocks.c
  ed25519_blowfish.c
  ed25519_crypto_api.h
  ed25519_fe25519.c
  ed25519_fe25519.h
  ed25519_ge25519.c
  ed25519_ge25519.h
  ed25519_hash.c
  ed25519_sc25519.c
  ed25519_sc25519.h
  ed25519_verify.c
  fwd.c
  fwd.h
  fwd-socks.c
  fwd-socks.h
  fwdui.c
  fwdui.h
  hosts.c
  hosts.h
  kex.c
  kex.h
  key.c
  key.h
  keyfiles.c
  keyfiles.h
  pkt.c
  pkt.h
  resource.h
  sftp.c
  sftp.h
  ssh.c
  ssh.h
  ttxssh.c
  ttxssh.def
  ttxssh.h
  ttxssh.rc
  ttxssh-version.h
  ttxssh-version.rc
  util.c
  util.h
  x11util.c
  x11util.h
  ${COMMON_SRC}
  )

target_sources(
  ttxssh
  PRIVATE
  ../../teraterm/teraterm/WSAAsyncGetAddrInfo.c
  ../../teraterm/common/ttcommon.h
  ../../teraterm/common/i18n.h
  ../../teraterm/common/ttlib.h
  ../../teraterm/common/ttlib.c
  ../../teraterm/common/dlglib.h
  ../../teraterm/common/dlglib.c
  ../../teraterm/common/dlglib_cpp.cpp
  ../../teraterm/common/dlglib_tmpl.cpp
  ../../teraterm/common/servicenames.h
  ../../teraterm/common/codeconv.cpp
  ../../teraterm/common/codeconv.h
  ../../teraterm/common/tipwin.cpp
  ../../teraterm/common/tipwin.h
  )

source_group(
  "common"
  REGULAR_EXPRESSION
  "teraterm\/common\/")

target_include_directories(
  ttxssh
  PRIVATE
  ../../teraterm/teraterm
  ../../teraterm/common
  ../matcher
  ../putty
  ${OPENSSL_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
)

set_target_properties(
  ttxssh
  PROPERTIES
  PREFIX ""
  )

target_compile_definitions(
  ttxssh
  PRIVATE
  $<$<CONFIG:Debug>:_CRTDBG_MAP_ALLOC>
  )
if(USE_UNICODE_API)
  target_compile_definitions(
    ttxssh
    PRIVATE
    UNICODE
    _UNICODE
    )
endif()


target_link_libraries(
  ttxssh
  libputty
  ttpcmn
  ${ZLIB_LIB}
  ${OPENSSL_LIB}
  #
  ws2_32
  dnsapi
  gdi32
  comdlg32
  delayimp
  )

### create puttyversion.h

set(PUTTY_ISS "${CMAKE_SOURCE_DIR}/libs/putty/windows/putty.iss")
if(NOT EXISTS ${PUTTY_ISS})
  message(FATAL_ERROR "${PUTTY_ISS} not found")
endif()

file(READ "${CMAKE_SOURCE_DIR}/libs/putty/windows/putty.iss" iis)

string(REGEX MATCH "VersionInfoTextVersion=([^\n]+)\n" _ ${iis})
set(version_str ${CMAKE_MATCH_1})
if("${version_str}" STREQUAL "")
  string(REGEX MATCH "AppVerName=([^\n]+)\n" _ ${iis})
  set(version_str ${CMAKE_MATCH_1})
endif()
if("${version_str}" STREQUAL "")
  set(version_str "unknown")
endif()
message("putty version=\"${version_str}\"")
FILE(
  WRITE ${CMAKE_CURRENT_BINARY_DIR}/puttyversion.h
  "#define PUTTYVERSION \"${version_str}\"\n"
  )

install(
  TARGETS ttxssh
  RUNTIME
  DESTINATION .
  )
