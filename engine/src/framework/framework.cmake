add_library(framework
        src/framework/ElementNew.h
        src/framework/ElementNew.cpp
        src/framework/DataPipeNew.cpp
        src/framework/ElementManager.h
        src/framework/ElementManager.cpp
        src/framework/ElementFactory.h
        src/framework/ElementFactory.cpp
        src/framework/Engine.h
        src/framework/Engine.cpp
        src/framework/PreModuleElement.h
        src/framework/PreModuleElement.cpp
        src/framework/PostModuleElement.h
        src/framework/PostModuleElement.cpp
        )
link_libraries(framework)

add_library(tracker_lib
        src/algorithm/tracker/HungarianState.cpp
        src/algorithm/tracker/sort.cpp
        src/algorithm/tracker/TrackerChannel.cpp
        )
link_libraries(tracker_lib)

# aux_source_directory(elements /src/element/)
# add_library(derivedelement elements)

add_library(derivedelement
    src/element/ActionElement.h
    src/element/ActionElement.cpp
    src/element/DecoderElement.h
    src/element/DecoderElement.cpp
    src/element/TrackerElement.h
    src/element/TrackerElement.cpp
    src/element/ReportElement.cpp
    )
link_libraries(derivedelement)

