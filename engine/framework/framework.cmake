add_library(framework
        framework/ElementNew.h
        framework/ElementNew.cpp
        framework/DataPipeNew.cpp
        framework/ElementManager.h
        framework/ElementManager.cpp
        framework/ElementFactory.h
        # framework/ElementFactory.cpp
        framework/Engine.h
        framework/Engine.cpp
        )
link_libraries(framework)