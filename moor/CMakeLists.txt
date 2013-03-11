set(include_HEADERS
  archive_writer.hpp
  archive_reader.hpp
  moor_build_config.hpp
  types.hpp
  archive.hpp
  archive_entry.hpp
  archive_iterator.hpp
  moor_utils.hpp
  archive_read_disk.hpp
  archive_write_disk.hpp
  )
set(libmoor_SOURCES
  archive_writer.cpp
  archive_reader.cpp
  memory_writer_callback.cpp
  types.cpp
  archive_entry.cpp
)

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX d)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif()


add_library(moor SHARED ${libmoor_SOURCES} ${INCLUDE_DIRECTORIES})
target_link_libraries(moor ${ADDITIONAL_LIBS})

add_library(moor_static STATIC ${libmoor_SOURCES} ${INCLUDE_DIRECTORIES})
set_target_properties(moor_static PROPERTIES COMPILE_DEFINITIONS MOOR_STATIC)
target_link_libraries(moor_static ${ADDITIONAL_LIBS})

if(NOT WIN32 OR CYGWIN)
  set_target_properties(moor_static PROPERTIES OUTPUT_NAME moor)
endif()

install(TARGETS moor moor_static
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(FILES ${include_HEADERS} DESTINATION include)
