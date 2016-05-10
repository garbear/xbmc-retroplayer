set(SOURCES GameFileAutoLauncher.cpp
            GameManager.cpp
            GameSettings.cpp)

set(HEADERS GameFileAutoLauncher.h
            GameManager.h
            GameSettings.h
            GameTypes.h)

core_add_library(games)
add_dependencies(games libcpluff)
