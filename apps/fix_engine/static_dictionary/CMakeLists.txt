 #  Created on: 26 Apr 2014
 #      Author: Wei Liew [wei@onesixeightsolutions.com]
 #
 #  Copyright Wei Liew 2012 - 2014.
 #  Distributed under the Boost Software License, Version 1.0.
 #  (See http://www.boost.org/LICENSE_1_0.txt)

MESSAGE("Auto generating FIX definition files ...")

find_program(xslt xsltproc)
set(BIN_DIR /bin)

macro (PROCESS_DICTIONARY macro_target in_file out_file)
    MESSAGE("Generating FIX definition file ${out_file}")
    add_custom_command (
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/gen/${out_file}
        COMMAND ${xslt} genXMLDefs.xslt ${in_file} > gen/temp.1.h
        COMMAND ${BIN_DIR}/cat gen/temp.1.h | ${BIN_DIR}/grep -v component > gen/temp.2.h
        COMMAND ${xslt} genFIXDefs.xslt gen/temp.2.h > gen/${out_file}
        COMMAND ${BIN_DIR}/rm gen/temp.*.h
        DEPENDS ${in_file} genXMLDefs.xslt genFIXDefs.xslt
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_custom_target(${macro_target} ALL DEPENDS gen/${out_file})
endmacro (PROCESS_DICTIONARY macro_target in_file out_file)

PROCESS_DICTIONARY(FIX40 ../cfgfiles/FIX40.xml FieldDefsGen40.h)
PROCESS_DICTIONARY(FIX41 ../cfgfiles/FIX41.xml FieldDefsGen41.h)
PROCESS_DICTIONARY(FIX42 ../cfgfiles/FIX42.xml FieldDefsGen42.h)
PROCESS_DICTIONARY(FIX43 ../cfgfiles/FIX43.xml FieldDefsGen43.h)
PROCESS_DICTIONARY(FIX44 ../cfgfiles/FIX44.xml FieldDefsGen44.h)

macro (PROCESS_V5_DICTIONARY macro_target tport_file in_file out_file)
    MESSAGE("Generating FIX definition file ${out_file}")
    add_custom_command (
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/gen/${out_file}
        # FIX Definition
        COMMAND ${xslt} genXMLDefs.xslt ${in_file} > gen/temp.1.h
        COMMAND ${BIN_DIR}/cat gen/temp.1.h | ${BIN_DIR}/grep -v component > gen/temp.2.h
        # FIX Transport Definition
        COMMAND ${xslt} genXMLDefs.xslt ${tport_file} > gen/temp.3.h
        COMMAND ${BIN_DIR}/cat gen/temp.3.h | ${BIN_DIR}/grep -v component > gen/temp.4.h
        
        COMMAND ${xslt} --stringparam transport_file gen/temp.4.h genFIXDefs.xslt gen/temp.2.h > gen/${out_file}
        COMMAND ${BIN_DIR}/rm gen/temp.*.h
        DEPENDS ${in_file} genXMLDefs.xslt genFIXDefs.xslt
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_custom_target(${macro_target} ALL DEPENDS gen/${out_file})
endmacro (PROCESS_V5_DICTIONARY macro_target in_file out_file)

PROCESS_V5_DICTIONARY(FIX50    ../cfgfiles/FIXT11.xml ../cfgfiles/FIX50.xml    FieldDefsGen50.h)
PROCESS_V5_DICTIONARY(FIX50SP1 ../cfgfiles/FIXT11.xml ../cfgfiles/FIX50SP1.xml FieldDefsGen50SP1.h)
PROCESS_V5_DICTIONARY(FIX50SP2 ../cfgfiles/FIXT11.xml ../cfgfiles/FIX50SP2.xml FieldDefsGen50SP2.h)

#add_subdirectory(test)
