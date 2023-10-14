import QtQuick 2.0
import org.SfietKonstantin.weatherfish 1.0
import "../components"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Set Location")

    property CityManager cityManager

    property string currentCity: (cityManager !== null) &&
        (cityManager.cities.length > 0) ? (cityManager.cities[0].name + ", " +  cityManager.cities[0].country ) : '' 

    Column {
        id: column
        anchors.fill: parent
        anchors.margins: styler.themePaddingSmall
        spacing: styler.themePaddingLarge

        CitySearchModel {
            id: citymodel
        }

        LabelPL {
            text: page.currentCity
            visible: page.currentCity !== ""
        }

        SearchFieldPL {
            id: searchField
            width: parent.width
            onTextChanged: {
                citymodel.search(text)
            }
        }

        ListViewPL {
            id: listView
            model: citymodel
            height: app.height - searchField.height - styler.themePaddingLarge
            width: parent.width

            delegate: ListItemPL {
                contentHeight: styler.themeItemSizeSmall

                onClicked: {
                    console.log("Setting city to ", model.identifier);
                    cityManager.removeAllCities();
                    cityManager.addCity(model.identifier, model.name, model.state,
                                        model.country, model.countryCode,
                                        model.longitude, model.latitude)
                    DaemonInterfaceInstance.reloadCities();
                    app.pages.pop();
                }

                Row {
                    anchors.left: parent.left; anchors.leftMargin: styler.themePaddingMedium
                    anchors.top: parent.top; anchors.bottom: parent.bottom
                    spacing: styler.themePaddingMedium
                    LabelPL {
                        width: page.width/4
                        anchors.verticalCenter: parent.verticalCenter
                        text: model.name
                    }
                    LabelPL {
                        width: page.width/4
                        anchors.verticalCenter: parent.verticalCenter
                        text: model.state
                    }
                    LabelPL {
                        width: page.width/4
                        anchors.verticalCenter: parent.verticalCenter
                        text: model.country
                    }
                }
            }

            BusyIndicatorPL {
                running: visible
                anchors.centerIn: parent
                visible: citymodel.status === CitySearchModel.Loading ? true : false
            }
        }
    }
}
