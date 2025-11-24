add_executable(help_printer1 help_printer1.cpp)
target_link_libraries(help_printer1 Boost::program_options)

add_executable(help_printer2 help_printer2.cpp)
target_link_libraries(help_printer2 Boost::program_options)

#add_executable(completer completer.cpp)
#target_link_libraries(completer Boost::program_options)

add_executable(SpiritParser SpiritParser.cpp)

add_executable(printer printer.cpp)