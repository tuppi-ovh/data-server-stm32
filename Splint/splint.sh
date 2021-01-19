#!/bin/bash

# parameters 
SPLINT_COMMAND="../splint/src/splint"
SPLINT_HTML_COMMAND="python3 ../splint-htmlreport/splint-htmlreport.py"
SPLINT_INCLUDES="-IInc/ -ISplint/"
SPLINT_DEFINES="-DSTM32F100xB"
SPLINT_CSV="Splint/splint.csv"
SPLINT_SOURCES="Src/*.c Src/*.cpp"
SPLINT_OPTIONS="+csvoverwrite -csv $SPLINT_CSV"
SPLINT_HTML="Splint/html"

# clean up before execution
if [[ -d "$SPLINT_HTML" ]]
then 
  rm -rf $SPLINT_HTML
fi
mkdir $SPLINT_HTML

# execute 
set +e
$SPLINT_COMMAND $SPLINT_INCLUDES $SPLINT_DEFINES $SPLINT_OPTIONS $SPLINT_SOURCES
set -e

# format output
$SPLINT_HTML_COMMAND --file $SPLINT_CSV --title "Data Server STM32" --report-dir $SPLINT_HTML --source-dir .