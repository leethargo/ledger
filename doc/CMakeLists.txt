# The following will be generated or updated when the 'doc' target is built:
# • user guide and man pages: if BUILD_DOCS is set
# • HTML versions of the above: if BUILD_DOCS and BUILD_WEB_DOCS are set
# • Doxygen / reference documentation: if USE_DOXYGEN is set

########################################################################

configure_file(
  ${PROJECT_SOURCE_DIR}/doc/version.texi.in
  ${PROJECT_BINARY_DIR}/doc/version.texi)

if (USE_DOXYGEN)
  find_package(Doxygen)
  if (NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Could not find doxygen. Reference documentation cannot be built.")
  endif()

  configure_file(Doxyfile.in Doxyfile @ONLY)

  # see INPUT/FILE_PATTERNS in Doxyfile.in
  file(GLOB doxygen_input_files ${CMAKE_SOURCE_DIR}/src/*.h)

  add_custom_command(OUTPUT html/index.html
    COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
    DEPENDS Doxyfile ${doxygen_input_files}
    COMMENT "Building doxygen documentation")
  add_custom_target(doc.doxygen DEPENDS html/index.html)
else()
  add_custom_target(doc.doxygen)
endif()

########################################################################

# BUILD_WEB_DOCS implies BUILD_DOCS
if (BUILD_WEB_DOCS)
  set(BUILD_DOCS 1)
endif()

if (BUILD_DOCS)
  find_program(MAKEINFO makeinfo)
  find_program(TEXI2PDF texi2pdf)
  find_program(TEX tex)
  find_program(MAN2HTML man2html)
  find_program(GROFF groff)
  set(ledger_info_files ledger3.texi ledger-mode.texi)

  if (NOT MAKEINFO)
    message(WARNING "Could not find makeinfo. Info version of documentation cannot be built.")
  endif()

  if (NOT TEXI2PDF OR NOT TEX)
    message(WARNING "Could not find texi2pdf or tex. PDF version of documentation will not be built.")
  endif()
endif()

########################################################################

foreach(file ${ledger_info_files})
  get_filename_component(file_base ${file} NAME_WE)

  if (MAKEINFO)
    add_custom_command(OUTPUT ${file_base}.info
      COMMAND makeinfo --force --no-split -o ${file_base}.info ${CMAKE_CURRENT_SOURCE_DIR}/${file}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file}
      VERBATIM)
    list(APPEND ledger_doc_files ${file_base}.info)
  endif()

  if (BUILD_WEB_DOCS AND MAKEINFO)
    add_custom_command(OUTPUT ${file_base}.html
      COMMAND makeinfo --force --html --no-split -o ${file_base}.html ${CMAKE_CURRENT_SOURCE_DIR}/${file}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file}
      VERBATIM)
    list(APPEND ledger_doc_files ${file_base}.html)
  endif()

  if (TEXI2PDF AND TEX)
    if (BUILD_A4_PDF)
      set(papersize --texinfo=@afourpaper)
    endif()
    add_custom_command(OUTPUT ${file_base}.pdf
      COMMAND texi2pdf ${papersize} -b -q -o ${file_base}.pdf ${CMAKE_CURRENT_SOURCE_DIR}/${file}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file}
      VERBATIM)
    list(APPEND ledger_doc_files ${file_base}.pdf)
  endif()
endforeach()

########################################################################

if (BUILD_WEB_DOCS)
  include(FindUnixCommands)
  if (NOT BASH)
    message(FATAL_ERROR "Could not find bash. Unable to build documentation.")
  endif()
  if (MAN2HTML)
    add_custom_command(OUTPUT ledger.1.html
      COMMAND ${BASH} -c "man2html ${CMAKE_CURRENT_SOURCE_DIR}/ledger.1 | tail -n+3 > ledger.1.html"
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ledger.1
      VERBATIM)
    list(APPEND ledger_doc_files ledger.1.html)
  elseif(GROFF)
    add_custom_command(OUTPUT ledger.1.html
      COMMAND ${BASH} -c "groff -mandoc -Thtml ${CMAKE_CURRENT_SOURCE_DIR}/ledger.1 > ledger.1.html"
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ledger.1
      VERBATIM)
    list(APPEND ledger_doc_files ledger.1.html)
  else()
    message(FATAL_ERROR "Could not find man2html or groff. HTML version of man page cannot be built.")
  endif()
endif(BUILD_WEB_DOCS)

########################################################################

add_custom_target(doc DEPENDS ${ledger_doc_files} doc.doxygen)

########################################################################

include(GNUInstallDirs)

if (CMAKE_INSTALL_MANDIR)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ledger.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT doc)
endif(CMAKE_INSTALL_MANDIR)

foreach(file ${ledger_doc_files})
  get_filename_component(file_ext ${file} EXT)

  if(file_ext STREQUAL ".info")
    if(CMAKE_INSTALL_INFODIR)
      install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${file}
        DESTINATION ${CMAKE_INSTALL_INFODIR} COMPONENT doc)
    endif()
  elseif(CMAKE_INSTALL_DOCDIR)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${file}
      DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT doc)
  endif()
endforeach()
