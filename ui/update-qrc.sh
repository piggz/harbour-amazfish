#!/bin/bash

declare -a platforms=("kirigami" "uuitk" "silica" "qtcontrols")

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
        for i in $(find qml/custom-icons/ -type f -name '*.png'|sort); do
            echo "        <file>$i</file>";
        done
        for i in $(find qml/activity-icons/ -type f -name '*.png'|sort); do
            echo "        <file>$i</file>";
        done
        for i in $(find qml/page-icons/ -type f -name '*.png'|sort); do
            echo "        <file>$i</file>";
        done
        echo "    </qresource>"
        echo "</RCC>"
    ) > icons.qrc

    # Pictures, kept apart from the QML code: rcc turns them into a large
    # source file, which then only has to be rebuilt when a picture changes.
    (
        echo "<RCC>"
        echo "    <qresource prefix=\"/\">"
        echo "        <file>icons/172x172/harbour-amazfish-ui.png</file>"
        # LC_ALL=C: the same order whatever the user's locale is
        for i in $(find qml/pics/ -type f -name '*.png'|LC_ALL=C sort); do
            echo "        <file>$i</file>";
        done
        echo "    </qresource>"
        echo "</RCC>"
    ) > images.qrc
