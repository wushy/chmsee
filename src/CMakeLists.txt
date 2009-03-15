include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CHMSEE_INCLUDE_DIRS}
  )
add_definitions(
  -Wall
  ${CHMSEE_CFLAGS_OTHER}
  )
add_custom_command(
  OUTPUT marshal.h
  COMMAND ${GLIB_GENMARSHAL} --prefix=marshal ${CMAKE_CURRENT_SOURCE_DIR}/marshal.list --header > marshal.h
  DEPENDS marshal.list
  )

add_custom_command(
  OUTPUT marshal.c
  COMMAND ${GLIB_GENMARSHAL} --prefix=marshal ${CMAKE_CURRENT_SOURCE_DIR}/marshal.list --body > marshal.c
  DEPENDS marshal.list
  )

link_directories(
  ${CHMSEE_LIBRARY_DIRS}
  )

set(CHMSEE_SRCS
  booktree.c chmsee.c gecko_utils.cpp html.c main.c
  setup.c startup.c ui_bookmarks.c utils.c booktree.h chmsee.h
  gecko_utils.h html.h setup.h startup.h ui_bookmarks.h
  utils.h marshal.h marshal.c
  ihtml.h ihtml.c
  )
set(CHMSEE_SRCS
  ${CHMSEE_SRCS}
  utils/utils.h utils/utils.c)
set(CHMSEE_SRCS
  ${CHMSEE_SRCS}
  models/bookmarks.h models/bookmarks.c
  models/chmfile.h models/chmfile.c
  models/hhc.h models/hhc.c
  models/link.h models/link.c
  models/ichmfile.h models/ichmfile.c
  models/chmfile-factory.h models/chmfile-factory.c
  )

add_executable(chmsee ${CHMSEE_SRCS})

target_link_libraries(chmsee
  ${CHMSEE_LIBRARIES}
  chm
  gcrypt
  )

install(TARGETS chmsee DESTINATION bin)
