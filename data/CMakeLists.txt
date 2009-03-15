add_custom_command(
	OUTPUT chmsee.desktop
	COMMAND intltool-merge -d ${CMAKE_SOURCE_DIR}/po ${CMAKE_CURRENT_SOURCE_DIR}/chmsee.desktop.in chmsee.desktop
	DEPENDS chmsee.desktop.in
	)
add_custom_command(
	OUTPUT default-prefs.js
	COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/default-prefs-common.js 
		${CMAKE_CURRENT_SOURCE_DIR}/default-prefs-${GECKO_FLAVOUR}.js > default-prefs.js
	DEPENDS default-prefs-common.js default-prefs-${GECKO_FLAVOUR}.js
	)
add_custom_target(driver ALL DEPENDS chmsee.desktop default-prefs.js)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/chmsee.desktop DESTINATION share/applications)
install(FILES chmsee.mime chmsee.keys DESTINATION share/mime-info)
install(FILES chmsee-icon.png DESTINATION share/pixmaps)
install(FILES
	book-closed.png book-open.png chmsee-icon.png chmsee-splash.png
	helpdoc.png hide-pane.png show-pane.png chmsee.glade 
	${CMAKE_CURRENT_BINARY_DIR}/default-prefs.js
	DESTINATION
	share/chmsee)
