add_executable(example1 example1.cpp)
target_link_libraries(example1 Boost::program_options)

add_executable(example2 example2.cpp)
target_link_libraries(example2 Boost::program_options)

#add_executable(completer completer.cpp)
#target_link_libraries(completer Boost::program_options)

add_executable(SpiritParser SpiritParser.cpp)