import QtQuick 2.0
//import QtGraphicalEffects 1.0
import Sailfish.Silica 1.0
import org.SfietKonstantin.weatherfish 1.0

Page {
    id: container
    property City city

    onCityChanged: {
        currentWeather.city = container.city
        currentWeather.refresh()
    }

    CurrentWeather {
        id: currentWeather
    }

    Rectangle {
        visible: container.city != null
        anchors {
            top: cityInfo.top
            topMargin: - 2 * Theme.paddingLarge
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        gradient: Gradient {
            GradientStop { position: 0; color: Theme.rgba("black", 0) }
            GradientStop { position: 0.2; color: Theme.rgba("black", 0.5) }
            GradientStop { position: 1; color: Theme.rgba("black", 0.8) }
        }
    }

    Column {
        id: cityInfo
        anchors {
            left: parent.left
            leftMargin: Theme.paddingLarge
            right: parent.right
            rightMargin: Theme.paddingLarge
            bottom: parent.bottom
            bottomMargin: Theme.paddingLarge
        }
        spacing: Theme.paddingMedium

        Item {
            anchors {
                left: parent.left
                leftMargin: Theme.paddingLarge
                right: parent.right
                rightMargin: Theme.paddingLarge
            }
            height: temperature.height

            Image {
                source: currentWeather.icon
                anchors.bottom: parent.bottom
            }

            Label {
                id: temperature
                text: currentWeather.temperature
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                font.pixelSize: Theme.fontSizeExtraLarge * 3
            }
        }


        Label {
            anchors {
                left: parent.left
                right: parent.right
            }

            font.pixelSize: Theme.fontSizeExtraLarge
            text: city != null ? city.name : ""

        }

    }

}
