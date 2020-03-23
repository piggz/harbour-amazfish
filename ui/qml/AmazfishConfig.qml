pragma Singleton
import QtQuick 2.0
import Nemo.Configuration 1.0


Item {
    readonly property string group: "/uk/co/piggz/amazfish/"

    property alias pairedAddress:                   pairedAddress.value
    property alias pairedName:                      pairedName.value
    property alias authKey:                         authKey.value

    property alias appNotifyConnect:                appNotifyConnect.value
    property alias appRefreshWeather:               appRefreshWeather.value
    property alias appAutoSyncData:                 appAutoSyncData.value
    property alias appNotifyLowBattery:             appNotifyLowBattery.value
    property alias appRefreshCalendar:              appRefreshCalendar.value

    property alias deviceLanguage:                  deviceLanguage.value
    property alias deviceDateFormat:                deviceDateFormat.value
    property alias deviceTimeFormat:                deviceTimeFormat.value
    property alias deviceDistanceUnit:              deviceDistanceUnit.value
    property alias deviceDisconnectNotification:    deviceDisconnectNotification.value

    property alias deviceDisplayStatus:             deviceDisplayStatus.value
    property alias deviceDisplayActivity:           deviceDisplayActivity.value
    property alias deviceDisplayWeather:            deviceDisplayWeather.value
    property alias deviceDisplayAlarm:              deviceDisplayAlarm.value
    property alias deviceDisplayTimer:              deviceDisplayTimer.value
    property alias deviceDisplayCompass:            deviceDisplayCompass.value
    property alias deviceDisplaySettings:           deviceDisplaySettings.value
    property alias deviceDisplayAliPay:             deviceDisplayAliPay.value
    property alias deviceDisplayWeathershortcut:    deviceDisplayWeathershortcut.value
    property alias deviceDisplayAliPayShortcut:     deviceDisplayAliPayShortcut.value

    property alias lastActivitySync:                lastActivitySync.value
    property alias lastSportSync:                   lastSportSync.value

    property alias profileName:                     profileName.value
    property alias profileDOB:                      profileDOB.value
    property alias profileGender:                   profileGender.value
    property alias profileHeight:                   profileHeight.value
    property alias profileWeight:                   profileWeight.value
    property alias profileWearLocation:             profileWearLocation.value
    property alias profileFitnessGoal:              profileFitnessGoal.value
    property alias profileAlertFitnessGoal:         profileAlertFitnessGoal.value
    property alias profileAllDayHRM:                profileAllDayHRM.value
    property alias profileHRMSleepSupport:          profileHRMSleepSupport.value
    property alias profileDisplayOnLiftWrist:       profileDisplayOnLiftWrist.value


    ConfigurationValue {
        id: pairedAddress
        key: group + "pairedAddress"
        defaultValue: ""
    }

    ConfigurationValue {
        id: pairedName
        key: group + "pairedName"
        defaultValue: ""
    }

    ConfigurationValue {
        id: authKey
        key: group + "device/authkey"
        defaultValue: ""
    }

    ConfigurationValue {
        id: appNotifyConnect
        key: group + "app/notifyconnect"
        defaultValue: true
    }

    ConfigurationValue {
        id: appRefreshWeather
        key: group + "app/refreshweather"
        defaultValue: 60
    }

    ConfigurationValue {
        id: appAutoSyncData
        key: group + "app/autosyncdata"
        defaultValue: true
    }

    ConfigurationValue {
        id: appNotifyLowBattery
        key: group + "app/notifylowbattery"
        defaultValue: false
    }

    ConfigurationValue {
        id: appRefreshCalendar
        key: group + "app/refreshcalendar"
        defaultValue: 60
    }

    ConfigurationValue {
        id: deviceLanguage
        key: group + "device/language"
        defaultValue: 0
    }

    ConfigurationValue {
        id: deviceDateFormat
        key: group + "device/dateformat"
        defaultValue: 0
    }

    ConfigurationValue {
        id: deviceTimeFormat
        key: group + "device/timeformat"
        defaultValue: 0
    }

    ConfigurationValue {
        id: deviceDistanceUnit
        key: group + "device/distanceunit"
        defaultValue: 0
    }

    ConfigurationValue {
        id: deviceDisconnectNotification
        key: group + "device/disconnectnotification"
        defaultValue: false
    }

    ConfigurationValue {
        id: deviceDisplayStatus
        key: group + "device/displaystatus"
        defaultValue: true
    }

    ConfigurationValue {
        id: deviceDisplayActivity
        key: group + "device/displayactivity"
        defaultValue: true
    }


    ConfigurationValue {
        id: deviceDisplayWeather
        key: group + "device/displayweather"
        defaultValue: true
    }

    ConfigurationValue {
        id: deviceDisplayAlarm
        key: group + "device/displayalarm"
        defaultValue: true
    }

    ConfigurationValue {
        id: deviceDisplayTimer
        key: group + "device/displaytimer"
        defaultValue: true
    }

    ConfigurationValue {
        id: deviceDisplayCompass
        key: group + "device/displaycompass"
        defaultValue: true
    }

    ConfigurationValue {
        id: deviceDisplaySettings
        key: group + "device/displaysettings"
        defaultValue: true
    }


    ConfigurationValue {
        id: deviceDisplayAliPay
        key: group + "device/displayalipay"
        defaultValue: true
    }

    ConfigurationValue {
        id: deviceDisplayWeathershortcut
        key: group + "device/displayweathershortcut"
        defaultValue: true
    }

    ConfigurationValue {
        id: deviceDisplayAliPayShortcut
        key: group + "device/displayalipayshortcut"
        defaultValue: true
    }

    ConfigurationValue {
        id: lastActivitySync
        key: group + "device/lastactivitysyncmillis"
    }

    ConfigurationValue {
        id: lastSportSync
        key: group + "device/lastsportsyncmillis"
    }

    ConfigurationValue {
        id: profileName
        key: group + "profile/name"
        defaultValue: ""
    }

    ConfigurationValue {
        id: profileDOB
        key: group + "profile/dob"
        defaultValue: new Date().toString()
    }

    ConfigurationValue {
        id: profileGender
        key: group + "profile/gender"
        defaultValue: 0
    }

    ConfigurationValue {
        id: profileHeight
        key: group + "profile/height"
        defaultValue: 200
    }

    ConfigurationValue {
        id: profileWeight
        key: group + "profile/weight"
        defaultValue: 70
    }

    ConfigurationValue {
        id: profileWearLocation
        key: group + "profile/wearlocation"
        defaultValue: 0
    }

    ConfigurationValue {
        id: profileFitnessGoal
        key: group + "profile/fitnessgoal"
        defaultValue: "10000"
    }

    ConfigurationValue {
        id: profileAlertFitnessGoal
        key: group + "profile/alertfitnessgoal"
        defaultValue: false
    }

    ConfigurationValue {
        id: profileAllDayHRM
        key: group + "profile/alldayhrm"
        defaultValue: 0
    }

    ConfigurationValue {
        id: profileHRMSleepSupport
        key: group + "profile/hrmsleepsupport"
        defaultValue: true
    }

    ConfigurationValue {
        id: profileDisplayOnLiftWrist
        key: group + "profile/displayonliftwrist"
        defaultValue: false
    }
}
