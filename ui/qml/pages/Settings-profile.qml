import QtQuick 2.0
import Sailfish.Silica 1.0
import uk.co.piggz.amazfish 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    property string dob
    
    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

        // PullDownMenu and PushUpMenu must be declared in SilicaFlickable, SilicaListView or SilicaGridView


        // Tell SilicaFlickable the height of its content.
        contentHeight: column.height

        // Place our content in a Column.  The PageHeader is always placed at the top
        // of the page, followed by our content.
        Column {
            id: column
            x: Theme.horizontalPageMargin
            width: page.width - 2*Theme.horizontalPageMargin
            spacing: Theme.paddingLarge
            PageHeader {
                title: qsTr("Profile Settings")
            }

            SectionHeader {
                text: qsTr("Personal data")
            }

            TextField {
                id: fldName
                label: qsTr("Name:")
                placeholderText: qsTr("Name:")
                width: parent.width
            }

                Label {
                    id: lblDOB
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Birthday")
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeExtraSmall
                }
                Button {
                    id: btnDOB
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - lblDOB - Theme.paddingLarge
                    text: new Date(page.dob).toLocaleDateString();

                    onClicked: {
                        var dialog = pageStack.push(pickerComponent, {
                                                        date: new Date(AmazfishConfig.profileDOB)
                                                    })
                        dialog.accepted.connect(function() {
                            page.dob = dialog.date;
                        })
                    }

                    Component {
                        id: pickerComponent
                        DatePickerDialog {}
                    }
                }

            ComboBox {
                id: cboGender
                width: parent.width
                label: qsTr("Gender")

                menu: ContextMenu {
                    MenuItem { text: qsTr("Male") }
                    MenuItem { text: qsTr("Female") }
                }
            }
            Slider {
                id: sldHeight
                width: parent.width
                minimumValue: 50
                maximumValue: 250
                stepSize: 1
                label: qsTr("Height (cm): ") + value
            }
            Slider {
                id: sldWeight
                width: parent.width
                minimumValue: 50
                maximumValue: 200
                stepSize: 1
                label: qsTr("Weight (kg): ") + value
            }

            SectionHeader {
                text: qsTr("Personal preferences")
            }

            ComboBox {
                id: cboWearLocation
                width: parent.width
                label: qsTr("Wear Location")

                menu: ContextMenu {
                    MenuItem { text: qsTr("Left") }
                    MenuItem { text: qsTr("Right") }
                }
            }
            
            TextSwitch {
                id: swDisplayOnLiftWrist
                width: parent.width
                text: qsTr("Display on lift wrist")
            }

            Slider {
                id: sldFitnessGoal
                width: parent.width
                minimumValue: 5000
                maximumValue: 30000
                stepSize: 100
                label: qsTr("Goal (steps): ") + value
            }


            TextSwitch {
                id: swAlertOnGoal
                width: parent.width
                text: qsTr("Alert on fitness goal")
            }

            SectionHeader {
                text: qsTr("Heartrate usage")
            }

            TextSwitch {
                id: swHRMSleepSupport
                width: parent.width
                text: qsTr("Use HRM for Sleep Detection")
            }

            Slider {
                id: sldAllDayHRM
                width: parent.width

                minimumValue: 0
                maximumValue: 60
                stepSize: 1
                label: qsTr("All day HRM interval (minutes): ") + value
            }

            Separator {
                width: parent.width
                horizontalAlignment: Qt.AlignHCenter
                color: Theme.highlightColor
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Save Profile")
                onClicked: {
                    saveProfile();
                }
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
        swDisplayOnLiftWrist.checked = AmazfishConfig.profileDisplayOnLiftWrist;
        swHRMSleepSupport.checked = AmazfishConfig.profileHRMSleepSupport;
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
        AmazfishConfig.profileDisplayOnLiftWrist = swDisplayOnLiftWrist.checked;
        AmazfishConfig.profileHRMSleepSupport = swHRMSleepSupport.checked;

        tmrSetDelay.start();
    }

    Timer {
        //Allow data to sync
        id: tmrSetDelay
        repeat: false
        interval: 500
        running: false
        onTriggered: {
            DaemonInterfaceInstance.applyDeviceSetting(DaemonInterface.SETTING_USER_PROFILE);
            DaemonInterfaceInstance.applyDeviceSetting(DaemonInterface.SETTING_USER_GOAL);
            DaemonInterfaceInstance.applyDeviceSetting(DaemonInterface.SETTING_USER_ALERT_GOAL);
            DaemonInterfaceInstance.applyDeviceSetting(DaemonInterface.SETTING_USER_DISPLAY_ON_LIFT);
            DaemonInterfaceInstance.applyDeviceSetting(DaemonInterface.SETTING_USER_ALL_DAY_HRM);
            DaemonInterfaceInstance.applyDeviceSetting(DaemonInterface.SETTING_USER_HRM_SLEEP_DETECTION)
        }
    }
}
