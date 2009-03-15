add_custom_command(
	OUTPUT chmsee.desktop
	COMMAND intltool-merge -d ${CMAKE_SOURCE_DIR}/po ${CMAKE_CURRENT_SOURCE_DIR}/chmsee.desktop.in chmsee.desktop
	DEPENDS chmsee.desktop.in
	)
add_custom_target(driver ALL DEPENDS chmsee.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/chmsee.desktop DESTINATION share/applications)
