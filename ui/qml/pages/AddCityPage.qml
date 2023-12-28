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

    CurrentWeather {
        id: weather
    }

    function owmIconToLocal(omw) {
        const weatherIcons = {
          '01d': 'weather-clear.png',
          '01n': 'weather-clear-night.png',
          '02d': 'weather-few-clouds.png',
          '02n': 'weather-few-clouds-night.png',
          '03d': 'weather-clouds.png',
          '03n': 'weather-clouds-night.png',
          '04d': 'weather-many-clouds.png',
          '04n': 'weather-many-clouds.png', // Assuming night uses the same icon for "many clouds"
          '09d': 'weather-showers.png',
          '09n': 'weather-showers-night.png',
          '10d': 'weather-showers-scattered-day.png',
          '10n': 'weather-showers-scattered-night.png',
          '11d': 'weather-storm-day.png',
          '11n': 'weather-storm-night.png',
          '13d': 'weather-snow.png',
          '13n': 'weather-snow-scattered-night.png', // Assuming scattered snow at night for 13n
          '50d': 'weather-mist.png',
          '50n': 'weather-mist.png' // Assuming night uses the same icon for "mist"
        };
        return "../pics/" + weatherIcons[omw];
    }

    onCurrentCityChanged: {
        if ((cityManager == null) || (cityManager.cities.length <= 0)) {
            return
        }
        weather.setCity(cityManager.cities[0])
        weather.refresh()
    }

    Column {
        id: column
        anchors.fill: parent
        anchors.margins: styler.themePaddingSmall
        spacing: styler.themePaddingLarge

        CitySearchModel {
            id: citymodel
        }


        Row {
            width: parent.width - (2 * anchors.margins)
            LabelPL {
                text: page.currentCity
                width: parent.width * 0.75
            }

            LabelPL {
                id: temperatureLabel
                text: (weather.weatherIcon !== "") ? (Math.round(weather.temperature-273.15) + "˚C") : ""
                width: parent.width * 0.15

            }

            IconPL {
                source: (weather.weatherIcon !== "") ? owmIconToLocal(weather.weatherIcon) : ""
                height: temperatureLabel.height
            }

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
