cmake_minimum_required (VERSION 2.6)
project(midi_parser)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(midi_parser
  src
)

add_executable(midi_parser
  src/main.c
  src/midi.c
  src/midi_event.c
  src/sound.c
)

target_link_libraries(midi_parser
  m
)
