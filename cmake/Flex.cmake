macro(mimosa_flex name)
  add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${name}.hh ${CMAKE_CURRENT_SOURCE_DIR}/${name}.cc
    COMMAND flex --outfile=${CMAKE_CURRENT_SOURCE_DIR}/${name}.cc
                 --header-file=${CMAKE_CURRENT_SOURCE_DIR}/${name}.hh
                 ${CMAKE_CURRENT_SOURCE_DIR}/${name}.l
    DEPENDS ${name}.l
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endmacro(mimosa_flex)
