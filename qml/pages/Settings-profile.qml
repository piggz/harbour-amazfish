import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    property string dob

    ConfigurationValue {
        id: profileName
        key: "/uk/co/piggz/amazfish/profile/name"
        defaultValue: ""
    }

    ConfigurationValue {
        id: profileDOB
        key: "/uk/co/piggz/amazfish/profile/dob"
        defaultValue: new Date().toString()
    }

    ConfigurationValue {
        id: profileGender
        key: "/uk/co/piggz/amazfish/profile/gender"
        defaultValue: 0
    }

    ConfigurationValue {
        id: profileHeight
        key: "/uk/co/piggz/amazfish/profile/height"
        defaultValue: 200
    }

    ConfigurationValue {
        id: profileWeight
        key: "/uk/co/piggz/amazfish/profile/weight"
        defaultValue: 70
    }

    ConfigurationValue {
        id: profileWearLocation
        key: "/uk/co/piggz/amazfish/profile/wearlocation"
        defaultValue: 0
    }

    ConfigurationValue {
        id: profileFitnessGoal
        key: "/uk/co/piggz/amazfish/profile/fitnessgoal"
        defaultValue: 10000
    }

    ConfigurationValue {
        id: profileAlertFitnessGoal
        key: "/uk/co/piggz/amazfish/profile/alertfitnessgoal"
        defaultValue: false
    }

     ConfigurationValue {
        id: profileAllDayHRM
        key: "/uk/co/piggz/amazfish/profile/alldayhrm"
        defaultValue: 0
    }
     ConfigurationValue {
        id: profileDisplayOnLiftWrist
        key: "/uk/co/piggz/amazfish/profile/displayonliftwrist"
        defaultValue: false
    }
    
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

            TextField {
                id: fldName
                label: qsTr("Name:")
                placeholderText: qsTr("Name:")
                width: parent.width
            }
            Row {
                spacing: Theme.paddingLarge
                width: parent.width
                x: Theme.horizontalPageMargin

                Label {
                    id: lblDOB
                    text: "Birthday"
                }
                Button {
                    id: btnDOB
                    width: parent.width - lblDOB - Theme.paddingLarge
                    text: new Date(page.dob).toLocaleDateString();

                    onClicked: {
                        var dialog = pageStack.push(pickerComponent, {
                                                        date: new Date(profileDOB.value)
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
                text: qsTr("Alert on fitness goal")
            }
            
	Slider {
                id: sldALLDayHRM
                minimumValue: 0
                maximumValue: 60
                stepSize: 1
                label :qsTr("All day HRM interval")    
                
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
        fldName.text = profileName.value;
        dob = profileDOB.value
        cboGender.currentIndex = profileGender.value;
        sldHeight.value = profileHeight.value;
        sldWeight.value = profileWeight.value;
        cboWearLocation.currentIndex = profileWearLocation.value;
        sldFitnessGoal.value = profileFitnessGoal.value;
        swAlertOnGoal.checked = profileAlertFitnessGoal.value;
        //sldAllDayHRM.value = profileAllDayHRM.value;
        swDisplayOnLiftWrist.checked = profileDisplayOnLiftWrist.value;
    }
    function saveProfile() {
        profileName.value = fldName.text;
        profileDOB.value = dob;
        profileGender.value = cboGender.currentIndex;
        profileHeight.value = sldHeight.value;
        profileWeight.value = sldWeight.value;
        profileWearLocation.value = cboWearLocation.currentIndex;
        profileFitnessGoal.value = sldFitnessGoal.value;
        profileAlertFitnessGoal.value = swAlertOnGoal.checked;
        //profileAllDayHRM.value = sldAllDayHRM.value;
        profileDisplayOnLiftWrist.value = swDisplayOnLiftWrist.checked;
        
        tmrSetDelay.start();
    }

    Timer {
        //Allow data to sync
        id: tmrSetDelay
        repeat: false
        interval: 500
        running: false
        onTriggered: {
            DeviceInterface.miBandService().setUserInfo();
            DeviceInterface.miBandService().setFitnessGoal();
            DeviceInterface.miBandService().setAlertFitnessGoal();
            DeviceInterface.miBandService().setDisplayOnLiftWrist();
        }
    }
}
