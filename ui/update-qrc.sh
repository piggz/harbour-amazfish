#!/bin/bash

declare -a platforms=("kirigami" "uuitk" "silica" "qtcontrols")

for platform in ${platforms[@]}; do

    (
        echo "<RCC>"
        echo "    <qresource prefix=\"/\">"
        replace=platform.${platform}
        for i in $(find ./qml/components/ -path '*platform.'"$platform"'*' -name '*.qml'); do
            x=${i//$replace/platform}; 
            echo "<file alias=\"$x\">$i</file>"; 
        done 

        for i in $(find ./qml/components/ -maxdepth 1 -name '*.qml'); do
            echo "<file>$i</file>"; 
        done
        echo "    </qresource>"
        echo "</RCC>"
    ) > platform-${platform}.qrc

done
