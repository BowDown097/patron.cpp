add_library(patron)

set_target_properties(patron
    PROPERTIES
        CXX_STANDARD 26
        CXX_STANDARD_REQUIRED ON)

target_include_directories(patron PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(patron
    PRIVATE
        patron/services/moduleservice.cpp
    PUBLIC
        FILE_SET HEADERS FILES
            patron/commands/annotations.h
            patron/commands/commandinfo.h
            patron/modules/modulebase.h
            patron/services/moduleservice.h
            patron/utils/reflection.h)
