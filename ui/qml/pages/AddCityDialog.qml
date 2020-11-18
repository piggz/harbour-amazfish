import QtQuick 2.0
import org.SfietKonstantin.weatherfish 1.0
import "../components"
import "../components/platform"

DialogPL {
    id: page
    acceptText: qsTr("Set Location")

    property string selectedCity
    property CityManager cityManager
    canAccept: selectedCity != ""
    onAccepted: {
        cityManager.removeAllCities();
        cityManager.addCity(page.selectedCity, cityProperties.name, cityProperties.state,
                            cityProperties.country, cityProperties.countryCode,
                            cityProperties.longitude, cityProperties.latitude)
        DaemonInterfaceInstance.reloadCities();
    }

    QtObject {
        id: cityProperties
        property string name
        property string state
        property string country
        property string countryCode
        property string longitude
        property string latitude
    }

    CitySearchModel {
        id: model
    }

    ListViewPL {
        id: view
        currentIndex: -1
        anchors.fill: parent
        model: model
        header: Column {
            width: page.width

            SearchFieldPL {
                width: parent.width
                onTextChanged: {
                    model.search(text)
                    page.selectedCity = ""
                }
            }
        }
        delegate: MouseArea {
            id: background
            width: parent ? parent.width : Screen.width
            implicitHeight: styler.themeItemSizeSmall

            property bool selected: page.selectedCity == model.identifier
            onClicked: {
                page.selectedCity = model.identifier
                cityProperties.name = model.name
                cityProperties.state = model.state
                cityProperties.country = model.country
                cityProperties.countryCode = model.countryCode
                cityProperties.longitude = model.longitude
                cityProperties.latitude = model.latitude
            }
            Rectangle {
                id: content
                width: parent.width
                height: parent.height
                color: selected ? Qt.darker(styler.blockBg, 2) : "transparent"

                Row {
                    anchors.left: parent.left; anchors.leftMargin: styler.themePaddingMedium
                    anchors.top: parent.top; anchors.bottom: parent.bottom
                    spacing: styler.themePaddingMedium
                    LabelPL {
                        anchors.verticalCenter: parent.verticalCenter
                        color: background.selected ? styler.themeHighlightColor : styler.themePrimaryColor
                        text: model.name
                    }

                    LabelPL {
                        anchors.verticalCenter: parent.verticalCenter
                        color: background.selected ? styler.themeSecondaryHighlightColor
                                                   : styler.themeSecondaryColor
                        text: model.state
                    }
                    LabelPL {
                        anchors.verticalCenter: parent.verticalCenter
                        color: background.selected ? styler.themeSecondaryHighlightColor
                                                   : styler.themeSecondaryColor
                        text: model.country
                    }
                }

            }


        }

        BusyIndicatorPL {
            running: opacity != 0
            anchors.centerIn: parent
            opacity: model.status == CitySearchModel.Loading ? 1 : 0
        }
    }
}
