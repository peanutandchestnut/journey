set(SOURCES
    main.cpp
    dialog.cpp
   )

set(MOC_HEADERS
    dialog.h
   )

set(RESOURCES
   )

QT4_ADD_RESOURCES(RES_SOURCES ${RESOURCES})
QT4_WRAP_CPP(MOC_SRCS ${MOC_HEADERS})

add_executable(meta ${SOURCES} ${MOC_SRCS} ${RES_SOURCES} )
set_target_properties(meta PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(meta Qt4::QtGui Qt4::QtXml)
