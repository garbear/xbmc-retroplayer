set(SOURCES SteamLinkVideo.cpp)

set(HEADERS SteamLinkVideo.h)

core_add_library(steamlinkvideo)

if(ENABLE_INTERNAL_FFMPEG)
  add_dependencies(steamlinkvideo ffmpeg)
endif()
