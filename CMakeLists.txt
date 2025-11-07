add_library(patron)

set_target_properties(patron
    PROPERTIES
        CXX_STANDARD 26
        CXX_STANDARD_REQUIRED ON
        LINKER_LANGUAGE CXX)

target_include_directories(patron PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(patron
    PRIVATE
        patron/commands/command_info.cpp
        patron/modules/module_base.cpp
        patron/utils/strings.cpp
    PUBLIC
        FILE_SET HEADERS FILES
            patron/commands/annotations.h
            patron/commands/command_info.h
            patron/commands/type_reader.h
            patron/modules/module_base.h
            patron/results/command_error.h
            patron/results/command_result.h
            patron/results/result.h
            patron/services/module_service.h
            patron/utils/concepts.h
            patron/utils/reflection.h
            patron/utils/strings.h)
