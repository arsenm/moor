add_executable(test_compress test_compress.cpp)

if(CMAKE_COMPILER_IS_GNUCXX)
  add_definitions (-std=c++0x)
else()
  add_definitions(-DMOOR_STATIC)
endif()

include_directories(${PROJECT_SOURCE_DIR})
target_link_libraries(test_compress moor_static)
target_link_libraries(test_compress ${ADDITIONAL_LIBS})
#SET_TARGET_PROPERTIES(moor PROPERTIES SOVERSION ${SOVERSION})
