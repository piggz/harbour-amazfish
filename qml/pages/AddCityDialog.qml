import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Silica.theme 1.0
import org.SfietKonstantin.weatherfish 1.0

Dialog {
    id: page
    property string selectedCity
    property CityManager cityManager
    canAccept: selectedCity != ""
    onAccepted: {
        cityManager.removeAllCities();
        cityManager.addCity(page.selectedCity, cityProperties.name, cityProperties.state,
                            cityProperties.country, cityProperties.countryCode,
                            cityProperties.longitude, cityProperties.latitude)
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

    SilicaListView {
        id: view
        currentIndex: -1
        anchors.fill: parent
        model: model
        header: Column {
            width: page.width
            DialogHeader {
                acceptText: "Add a city"
            }

            SearchField {
                width: parent.width
                onTextChanged: {
                    model.search(text)
                    page.selectedCity = ""
                }
            }
        }
        delegate: BackgroundItem {
            id: background
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
            Row {
                anchors.left: parent.left; anchors.leftMargin: Theme.paddingMedium
                anchors.top: parent.top; anchors.bottom: parent.bottom
                spacing: Theme.paddingMedium
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    color: background.selected ? Theme.highlightColor : Theme.primaryColor
                    text: model.name
                }

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    color: background.selected ? Theme.secondaryHighlightColor
                                               : Theme.secondaryColor
                    text: model.state
                }
            }
        }

        BusyIndicator {
            running: opacity != 0
            anchors.centerIn: parent
            opacity: model.status == CitySearchModel.Loading ? 1 : 0

            Behavior on opacity {
                FadeAnimation {}
            }
        }
    }
}
