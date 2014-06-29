 #  Created on: 26 Apr 2014
 #      Author: Wei Liew [wei@onesixeightsolutuons.comm]
 #
 #  Copyright Wei Liew 2012 - 2014.
 #  Distributed under the Boost Software License, Version 1.0.
 #  (See http://www.boost.org/LICENSE_1_0.txt)
 
set (BIN_NAME fix_engine)
MESSAGE("Processing application build [${CMAKE_CURRENT_SOURCE_DIR}]")

add_subdirectory(message)
add_subdirectory(dictionary)
add_subdirectory(static_dictionary)

EXECUTE_PROCESS(
    COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -maxdepth 1 -name *.cpp
    OUTPUT_VARIABLE MY_SRC_FILES)
string(REPLACE "\n" ";" MY_ALL_SRC ${MY_SRC_FILES})

MESSAGE("File list [${MY_ALL_SRC}]")

add_executable (${BIN_NAME} 
                ${MY_ALL_SRC})
                 
target_link_libraries(${BIN_NAME} ${MY_COMMON_LIBS} ${MY_BOOST_LIBS})
install (TARGETS ${BIN_NAME} DESTINATION ${PROD_INSTALL_DIR}/bin)

