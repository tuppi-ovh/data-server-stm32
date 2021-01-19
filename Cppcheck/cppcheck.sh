#!/bin/bash

# parameters 
CPPCHECK_COMMAND="../../cppcheck/build/bin/cppcheck"
CPPCHECK_HTML_COMMAND="python3 ../../cppcheck/htmlreport/cppcheck-htmlreport"
CPPCHECK_INCLUDES="-I Inc/ -I Cppcheck/"
CPPCHECK_DEFINES="-D STM32F100xB"
CPPCHECK_XML="Cppcheck/cppcheck.xml"
CPPCHECK_SOURCES="Src/*.c Src/*.cpp"
CPPCHECK_OPTIONS="--xml --xml-version=2 --enable=information --check-config"
CPPCHECK_HTML="Cppcheck/html"

# clean up before execution
if [[ -d "$CPPCHECK_HTML" ]]
then 
  rm -rf $CPPCHECK_HTML
fi
mkdir $CPPCHECK_HTML

# execute 
$CPPCHECK_COMMAND $CPPCHECK_INCLUDES $CPPCHECK_DEFINES $CPPCHECK_OPTIONS $CPPCHECK_SOURCES 2> $CPPCHECK_XML

# format output
$CPPCHECK_HTML_COMMAND --file $CPPCHECK_XML --title "Data Server STM32" --report-dir $CPPCHECK_HTML --source-dir .