macro(add_copy_target TARGETNAME GLOBPAT DESTINATION)
	file(GLOB_RECURSE COPY_FILES RELATIVE "${PROJECT_SOURCE_DIR}/${GLOBPAT}" "${PROJECT_SOURCE_DIR}/${GLOBPAT}/*")
	add_custom_target(${TARGETNAME} ALL COMMENT "Copying ${GLOBPAT}")
	
	foreach(FILENAME ${COPY_FILES})
		set(SRC "${PROJECT_SOURCE_DIR}/${GLOBPAT}/${FILENAME}")
		set(DST "${PROJECT_BINARY_DIR}/${DESTINATION}/${FILENAME}")
		
		add_custom_command(
			TARGET ${TARGETNAME}
			COMMAND ${CMAKE_COMMAND} -E copy ${SRC} ${DST}
		)
	endforeach(FILENAME)
endmacro(add_copy_target)
