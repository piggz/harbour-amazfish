import QtQuick 2.0
import "../components/"
import "../components/platform"
import uk.co.piggz.amazfish 1.0

PagePL {
    id: page
    title: qsTr("Profile Settings")

    property string dob
    property date wristScheduleStart
    property date wristScheduleEnd

    Column {
        id: column
        width: parent.width
        anchors.top: parent.top
        anchors.margins: styler.themePaddingMedium
        spacing: styler.themePaddingLarge

        SectionHeaderPL {
            text: qsTr("Personal data")
        }

        TextFieldPL {
            id: fldName
            label: qsTr("Name:")
            placeholderText: qsTr("Name:")
            width: parent.width
        }

        LabelPL {
            id: lblDOB
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Birthday")

        }
        ButtonPL {
            id: btnDOB
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - lblDOB.width - styler.themePaddingLarge
            text: new Date(page.dob).toLocaleDateString();

            onClicked: {
                var profile_DOB_date = new Date(AmazfishConfig.profileDOB);
                var dialog = app.pages.push(pickerComponent, {
                                                date: !isNaN(profile_DOB_date) ? profile_DOB_date : new Date()
                                            })
                dialog.accepted.connect(function() {
                    page.dob = dialog.date;
                })
            }

            Component {
                id: pickerComponent
                DatePickerDialogPL {}
            }
        }

        ComboBoxPL {
            id: cboGender
            width: parent.width
            label: qsTr("Gender")

            model: ListModel {
                ListElement { itemText: qsTr("Male") }
                ListElement { itemText: qsTr("Female")}
            }
        }
        SliderPL {
            id: sldHeight
            width: parent.width
            minimumValue: 50
            maximumValue: 250
            stepSize: 1
            label: qsTr("Height (cm): ") + value
        }
        SliderPL {
            id: sldWeight
            width: parent.width
            minimumValue: 50
            maximumValue: 200
            stepSize: 1
            label: qsTr("Weight (kg): ") + value
        }

        SectionHeaderPL {
            text: qsTr("Personal preferences")
        }

        ComboBoxPL {
            id: cboWearLocation
            width: parent.width
            label: qsTr("Wear Location")

            model: ListModel {
                ListElement { itemText: qsTr("Left") }
                ListElement { itemText: qsTr("Right")}
            }
        }

        /*
        TextSwitchPL {
            id: swDisplayOnLiftWrist
            width: parent.width
            text: qsTr("Display on lift wrist")
        }
        */

        ComboBoxPL {
            id: cboDisplayLiftWrist
            width: parent.width
            label: qsTr("Display on lift wrist")

            model: ListModel {
                ListElement { itemText: qsTr("Off") }
                ListElement { itemText: qsTr("On")}
                ListElement { itemText: qsTr("Schedule")}
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: styler.themePaddingSmall
            ButtonPL {
                id: btnLiftWristScheduleStart
                text: new Date(page.wristScheduleStart).toTimeString();
                enabled: cboDisplayLiftWrist.currentIndex === 2

                onClicked: {
                    var profile_lift_start = new Date(AmazfishConfig.profileWristScheduleStart);
                    var dialog = app.pages.push(pickerComponentScheduleStart, {
                                                hour: profile_lift_start.getHours(),
                                                minute: profile_lift_start.getMinutes()
                                                });
                    dialog.accepted.connect(function() {
                        console.log(dialog.timeText, dialog.time, dialog.hour, dialog.minute);
                        page.wristScheduleStart = dialog.time;
                    })
                }

                Component {
                    id: pickerComponentScheduleStart
                    TimePickerDialogPL {}
                }
            }

            ButtonPL {
                id: btnLiftWristScheduleEnd
                text: new Date(page.wristScheduleEnd).toTimeString();
                enabled: cboDisplayLiftWrist.currentIndex === 2

                onClicked: {
                    var profile_lift_end = new Date(AmazfishConfig.profileWristScheduleEnd);
                    var dialog = app.pages.push(pickerComponentScheduleEnd, {
                                                    hour: profile_lift_end.getHours(),
                                                    minute: profile_lift_end.getMinutes()
                                                    });
                    dialog.accepted.connect(function() {
                        page.wristScheduleEnd = dialog.time;
                    })
                }

                Component {
                    id: pickerComponentScheduleEnd
                    TimePickerDialogPL {}
                }
            }
        }

        SliderPL {
            id: sldFitnessGoal
            width: parent.width
            minimumValue: 1000
            maximumValue: 30000
            stepSize: 100
            label: qsTr("Goal (steps): ") + value
        }


        TextSwitchPL {
            id: swAlertOnGoal
            width: parent.width
            text: qsTr("Alert on fitness goal")
        }

        SectionHeaderPL {
            text: qsTr("Heartrate usage")
        }

        TextSwitchPL {
            id: swHRMSleepSupport
            width: parent.width
            text: qsTr("Use HRM for Sleep Detection")
        }

        SliderPL {
            id: sldAllDayHRM
            width: parent.width

            minimumValue: 0
            maximumValue: 60
            stepSize: 1
            label: qsTr("All day HRM interval (minutes): ") + value
        }

        ButtonPL {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Save Profile")
            onClicked: {
                saveProfile();
            }
        }
        Timer {
            //Allow data to sync
            id: tmrSetDelay
            repeat: false
            interval: 500
            running: false
            onTriggered: {
                DaemonInterfaceInstance.applyDeviceSetting(Amazfish.SETTING_USER_PROFILE);
                DaemonInterfaceInstance.applyDeviceSetting(Amazfish.SETTING_USER_GOAL);
                DaemonInterfaceInstance.applyDeviceSetting(Amazfish.SETTING_USER_ALERT_GOAL);
                DaemonInterfaceInstance.applyDeviceSetting(Amazfish.SETTING_USER_DISPLAY_ON_LIFT);
                DaemonInterfaceInstance.applyDeviceSetting(Amazfish.SETTING_USER_ALL_DAY_HRM);
                DaemonInterfaceInstance.applyDeviceSetting(Amazfish.SETTING_USER_HRM_SLEEP_DETECTION)
                app.pages.pop();
            }
        }
    }

    Component.onCompleted: {
        fldName.text = AmazfishConfig.profileName;
        dob = AmazfishConfig.profileDOB;
        cboGender.currentIndex = AmazfishConfig.profileGender;
        sldHeight.value = AmazfishConfig.profileHeight;
        sldWeight.value = AmazfishConfig.profileWeight;
        cboWearLocation.currentIndex = AmazfishConfig.profileWearLocation;
        sldFitnessGoal.value = AmazfishConfig.profileFitnessGoal;
        swAlertOnGoal.checked = AmazfishConfig.profileAlertFitnessGoal;
        sldAllDayHRM.value = AmazfishConfig.profileAllDayHRM;
        cboDisplayLiftWrist.currentIndex = AmazfishConfig.profileDisplayOnLiftWrist;
        swHRMSleepSupport.checked = AmazfishConfig.profileHRMSleepSupport;
        wristScheduleStart = AmazfishConfig.profileWristScheduleStart;
        wristScheduleEnd = AmazfishConfig.profileWristScheduleEnd;
    }
    function saveProfile() {
        AmazfishConfig.profileName = fldName.text;
        AmazfishConfig.profileDOB = dob;
        AmazfishConfig.profileGender = cboGender.currentIndex;
        AmazfishConfig.profileHeight = sldHeight.value;
        AmazfishConfig.profileWeight = sldWeight.value;
        AmazfishConfig.profileWearLocation = cboWearLocation.currentIndex;
        AmazfishConfig.profileFitnessGoal = sldFitnessGoal.value;
        AmazfishConfig.profileAlertFitnessGoal = swAlertOnGoal.checked;
        AmazfishConfig.profileAllDayHRM = sldAllDayHRM.value;
        AmazfishConfig.profileDisplayOnLiftWrist = cboDisplayLiftWrist.currentIndex;
        AmazfishConfig.profileWristScheduleStart = wristScheduleStart;
        AmazfishConfig.profileWristScheduleEnd = wristScheduleEnd;
        AmazfishConfig.profileHRMSleepSupport = swHRMSleepSupport.checked;

        tmrSetDelay.start();
    }
}
