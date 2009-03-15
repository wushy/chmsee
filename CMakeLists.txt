project(chmsee)
cmake_minimum_required(VERSION 2.4.6 FATAL_ERROR)
set(PACKAGE_VERSION "1.0.4-SNAPSHOT" )
find_package(PkgConfig)
pkg_check_modules (CHMSEE REQUIRED gthread-2.0 glib-2.0 libxml-2.0 gtk+-2.0 libglade-2.0 libxul-embedding-unstable)

set(CHMSEE_BOOKMARK_FILE "chmsee_bookmarks")
set(CHMSEE_BOOKINFO_FILE "chmsee_bookinfo")
set(CHMSEE_NO_LINK "chmsee_no_link")
set(CHMSEE_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share/chmsee")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

find_program(GLIB_GENMARSHAL glib-genmarshal)
if(${GLIB_GENMARSHAL} STREQUAL "GLIB_GENMARSHAL-NOTFOUND")
  message(FATAL_ERROR "glib-genmarshal not found, check your glib-2.0 development environment")
endif()

find_library(LIBCHM chm)
if(${LIBCHM} STREQUAL "LIBCHM-NOTFOUND")
  message(FATAL_ERROR "libchm not found, you can find one in http://www.jedrea.com/chmlib/")
endif()

find_library(LIBGCRYPT gcrypt)
if(${LIBGCRYPT} STREQUAL "LIBGCRYPT-NOTFOUND")
  message(FATAL_ERROR "libgcrypt not found, you can find one in ftp://ftp.gnupg.org/gcrypt/libgcrypt/")
endif()

add_subdirectory(src)
add_subdirectory(data)
