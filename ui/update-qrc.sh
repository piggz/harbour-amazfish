#!/bin/bash

declare -a platforms=("kirigami" "uuitk" "silica" "qtcontrols")

export LC_ALL=C
IFS=$'\n'

for platform in ${platforms[@]}; do

    (
        echo "<RCC>"
        echo "    <qresource prefix=\"/\">"
        replace=platform.${platform}
        for i in $(find ./qml/components/ -path '*platform.'"$platform"'*' -name '*.qml'|sort); do
            x=${i//$replace/platform}; 
            echo "        <file alias=\"$x\">$i</file>";
        done

        for i in $(find ./qml/components/ -maxdepth 1 -type f|sort); do
            echo "        <file>$i</file>";
        done
        echo "    </qresource>"
        echo "</RCC>"
    ) > platform-${platform}.qrc

done


    (
        echo "<RCC>"
        echo "    <qresource prefix=\"/\">"
        echo "        <file>icons/172x172/harbour-amazfish-ui.png</file>"
        for i in $(find qml/ -type f -name '*.png'|sort); do
            echo "        <file>$i</file>";
        done
        echo "    </qresource>"
        echo "</RCC>"
    ) > icons.qrc
