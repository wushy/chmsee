include_directories(  
  ${CHMSEE_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src
  )
link_directories(
  ${CHMSEE_LIBRARY_DIRS}
  )
  
add_executable(test EXCLUDE_FROM_ALL
	test.c)
	
target_link_libraries(test
  ${CHMSEE_LIBRARIES}
  chmseelib
  )
	