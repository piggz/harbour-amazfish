import "../components/"
import "../components/platform"
import QtQuick 2.0
import QtQuick.Layouts 1.1

PageListPL {
    id: page

    function fncCovertSecondsToString(sec) {
        var iHours = Math.floor(sec / 3600);
        var iMinutes = Math.floor((sec - iHours * 3600) / 60);
        var iSeconds = Math.floor(sec - (iHours * 3600) - (iMinutes * 60));
        return (iHours > 0 ? iHours + "h " : "") + (iMinutes > 0 ? iMinutes + "m " : "") + iSeconds + "s";
    }

    function translateSportKind(sportkind) {
        var translations = {
            "Unknown": qsTr("Unknown"),
            "Activity": qsTr("Activity"),
            "LightSleep": qsTr("Light Sleep"),
            "DeepSleep": qsTr("Deep Sleep"),
            "NotWorn": qsTr("Not Worn"),
            "Running": qsTr("Running"),
            "Walking": qsTr("Walking"),
            "Swimming": qsTr("Swimming"),
            "Cycling": qsTr("Cycling"),
            "Treadmill": qsTr("Treadmill"),
            "Exercise": qsTr("Exercise"),
            "OpenSwimming": qsTr("Open Swimming"),
            "IndoorCycling": qsTr("Indoor Cycling"),
            "EllipticalTrainer": qsTr("Elliptical Trainer"),
            "JumpRope": qsTr("Jump Rope"),
            "Yoga": qsTr("Yoga"),
            "Soccer": qsTr("Soccer"),
            "RowingMachine": qsTr("Rowing Machine"),
            "Cricket": qsTr("Cricket"),
            "Basketball": qsTr("Basketball"),
            "PingPong": qsTr("Ping Pong"),
            "Badmington": qsTr("Badmington"),
            "StrengthTraining": qsTr("Strength Training"),
            "Hiking": qsTr("Hiking"),
            "Climbing": qsTr("Climbing"),
            "REMSleep": qsTr("REM Sleep"),
            "SleepAny": qsTr("Sleep Any"),
            "AwakeSleep": qsTr("Awake Sleep"),
            "Navigate": qsTr("Navigate"),
            "IndoorTrack": qsTr("Indoor Track"),
            "Handcycling": qsTr("Handcycling"),
            "EBike": qsTr("E Bike"),
            "BikeCommute": qsTr("Bike Commute"),
            "HandcyclingIndoor": qsTr("Handcycling Indoor"),
            "Transition": qsTr("Transition"),
            "FitnessEquipment": qsTr("Fitness Equipment"),
            "StairStepper": qsTr("Stair Stepper"),
            "Pilates": qsTr("Pilates"),
            "PoolSwim": qsTr("Pool Swim"),
            "Tennis": qsTr("Tennis"),
            "PlatformTennis": qsTr("Platform Tennis"),
            "TableTennis": qsTr("Table Tennis"),
            "AmericanFootball": qsTr("American Football"),
            "Training": qsTr("Training"),
            "Cardio": qsTr("Cardio"),
            "Breathwork": qsTr("Breathwork"),
            "IndoorWalking": qsTr("Indoor Walking"),
            "XcClassicSki": qsTr("XC Classic Ski"),
            "Skiing": qsTr("Skiing"),
            "Snowboarding": qsTr("Snowboarding"),
            "Rowing": qsTr("Rowing"),
            "Mountaineering": qsTr("Mountaineering"),
            "Multisport": qsTr("Multisport"),
            "Paddling": qsTr("Paddling"),
            "Flying": qsTr("Flying"),
            "Motorcycling": qsTr("Motorcycling"),
            "Boating": qsTr("Boating"),
            "Driving": qsTr("Driving"),
            "Golf": qsTr("Golf"),
            "HangGliding": qsTr("Hang Gliding"),
            "Hunting": qsTr("Hunting"),
            "Fishing": qsTr("Fishing"),
            "InlineSkating": qsTr("Inline Skating"),
            "RockClimbing": qsTr("Rock Climbing"),
            "ClimbIndoor": qsTr("Climb Indoor"),
            "Bouldering": qsTr("Bouldering"),
            "SailRace": qsTr("Sail Race"),
            "SailExpedition": qsTr("Sail Expedition"),
            "IceSkating": qsTr("Ice Skating"),
            "SkyDiving": qsTr("Sky Diving"),
            "Snowshoe": qsTr("Snowshoe"),
            "Snowmobiling": qsTr("Snowmobiling"),
            "StandUpPaddleboarding": qsTr("Stand Up Paddleboarding"),
            "Surfing": qsTr("Surfing"),
            "Wakeboarding": qsTr("Wakeboarding"),
            "WaterSkiing": qsTr("Water Skiing"),
            "Kayaking": qsTr("Kayaking"),
            "Rafting": qsTr("Rafting"),
            "Windsurfing": qsTr("Windsurfing"),
            "Kitesurfing": qsTr("Kitesurfing"),
            "Tactical": qsTr("Tactical"),
            "Jumpmaster": qsTr("Jumpmaster"),
            "Boxing": qsTr("Boxing"),
            "FloorClimbing": qsTr("Floor Climbing"),
            "Baseball": qsTr("Baseball"),
            "Softball": qsTr("Softball"),
            "SoftballSlowPitch": qsTr("Softball Slow Pitch"),
            "Shooting": qsTr("Shooting"),
            "AutoRacing": qsTr("Auto Racing"),
            "WinterSport": qsTr("Winter Sport"),
            "Grinding": qsTr("Grinding"),
            "HealthSnapshot": qsTr("Health Snapshot"),
            "Marine": qsTr("Marine"),
            "Hiit": qsTr("HIIT"),
            "VideoGaming": qsTr("Video Gaming"),
            "Racket": qsTr("Racket"),
            "Pickleball": qsTr("Pickleball"),
            "Padel": qsTr("Padel"),
            "Squash": qsTr("Squash"),
            "Racquetball": qsTr("Racquetball"),
            "PushWalkSpeed": qsTr("Push Walk Speed"),
            "IndoorPushWalkSpeed": qsTr("Indoor Push Walk Speed"),
            "PushRunSpeed": qsTr("Push Run Speed"),
            "IndoorPushRunSpeed": qsTr("Indoor Push Run Speed"),
            "Meditation": qsTr("Meditation"),
            "ParaSport": qsTr("Para Sport"),
            "DiscGolf": qsTr("Disc Golf"),
            "UltimateDisc": qsTr("Ultimate Disc"),
            "TeamSport": qsTr("Team Sport"),
            "Rugby": qsTr("Rugby"),
            "Hockey": qsTr("Hockey"),
            "Lacrosse": qsTr("Lacrosse"),
            "Volleyball": qsTr("Volleyball"),
            "WaterTubing": qsTr("Water Tubing"),
            "Wakesurfing": qsTr("Wakesurfing"),
            "MixedMartialArts": qsTr("Mixed Martial Arts"),
            "Dance": qsTr("Dance"),
            "MountainHike": qsTr("Mountain Hike"),
            "CrossTrainer": qsTr("Cross Trainer"),
            "FreeTraining": qsTr("Free Training"),
            "DynamicCycle": qsTr("Dynamic Cycle"),
            "Kickboxing": qsTr("Kickboxing"),
            "FitnessExercises": qsTr("Fitness Exercises"),
            "Crossfit": qsTr("Crossfit"),
            "FunctionalTraining": qsTr("Functional Training"),
            "PhysicalTraining": qsTr("Physical Training"),
            "Taekwondo": qsTr("Taekwondo"),
            "TaeBo": qsTr("Tae Bo"),
            "CrossCountryRunning": qsTr("Cross Country Running"),
            "Karate": qsTr("Karate"),
            "Fencing": qsTr("Fencing"),
            "CoreTraining": qsTr("Core Training"),
            "Kendo": qsTr("Kendo"),
            "HorizontalBar": qsTr("Horizontal Bar"),
            "ParallelBar": qsTr("Parallel Bar"),
            "Cooldown": qsTr("Cooldown"),
            "CrossTraining": qsTr("Cross Training"),
            "SitUps": qsTr("Sit Ups"),
            "FitnessGaming": qsTr("Fitness Gaming"),
            "AerobicExercise": qsTr("Aerobic Exercise"),
            "Rolling": qsTr("Rolling"),
            "Flexibility": qsTr("Flexibility"),
            "Gymnastics": qsTr("Gymnastics"),
            "TrackAndField": qsTr("Track And Field"),
            "PushUps": qsTr("Push Ups"),
            "BattleRope": qsTr("Battle Rope"),
            "SmithMachine": qsTr("Smith Machine"),
            "PullUps": qsTr("Pull Ups"),
            "Plank": qsTr("Plank"),
            "Javelin": qsTr("Javelin"),
            "LongJump": qsTr("Long Jump"),
            "HighJump": qsTr("High Jump"),
            "Trampoline": qsTr("Trampoline"),
            "Dumbbell": qsTr("Dumbbell"),
            "BellyDance": qsTr("Belly Dance"),
            "JazzDance": qsTr("Jazz Dance"),
            "LatinDance": qsTr("Latin Dance"),
            "Ballet": qsTr("Ballet"),
            "StreetDance": qsTr("Street Dance"),
            "Zumba": qsTr("Zumba"),
            "RollerSkating": qsTr("Roller Skating"),
            "MartialArts": qsTr("Martial Arts"),
            "TaiChi": qsTr("Tai Chi"),
            "HulaHooping": qsTr("Hula Hooping"),
            "DiscSports": qsTr("Disc Sports"),
            "Darts": qsTr("Darts"),
            "Archery": qsTr("Archery"),
            "HorseRiding": qsTr("Horse Riding"),
            "KiteFlying": qsTr("Kite Flying"),
            "Swing": qsTr("Swing"),
            "Stairs": qsTr("Stairs"),
            "MindAndBody": qsTr("Mind And Body"),
            "Wrestling": qsTr("Wrestling"),
            "Kabaddi": qsTr("Kabaddi"),
            "Karting": qsTr("Karting"),
            "Billiards": qsTr("Billiards"),
            "Bowling": qsTr("Bowling"),
            "Shuttlecock": qsTr("Shuttlecock"),
            "Handball": qsTr("Handball"),
            "Dodgeball": qsTr("Dodgeball"),
            "AustralianFootball": qsTr("Australian Football"),
            "Lacross": qsTr("Lacross"),
            "Shot": qsTr("Shot"),
            "BeachSoccer": qsTr("Beach Soccer"),
            "BeachVolleyball": qsTr("Beach Volleyball"),
            "Gateball": qsTr("Gateball"),
            "SepakTakraw": qsTr("Sepak Takraw"),
            "Sailing": qsTr("Sailing"),
            "JetSkiing": qsTr("Jet Skiing"),
            "Skating": qsTr("Skating"),
            "IceHockey": qsTr("Ice Hockey"),
            "Curling": qsTr("Curling"),
            "CrossCountrySkiing": qsTr("Cross Country Skiing"),
            "SnowSports": qsTr("Snow Sports"),
            "Luge": qsTr("Luge"),
            "Skateboarding": qsTr("Skateboarding"),
            "Parachuting": qsTr("Parachuting"),
            "Parkour": qsTr("Parkour"),
            "IndoorRunning": qsTr("Indoor Running"),
            "OutdoorRunning": qsTr("Outdoor Running"),
            "OutdoorWalking": qsTr("Outdoor Walking"),
            "OutdoorCycling": qsTr("Outdoor Cycling"),
            "AerobicCombo": qsTr("Aerobic Combo"),
            "Aerobics": qsTr("Aerobics"),
            "AirWalker": qsTr("Air Walker"),
            "ArtisticSwimming": qsTr("Artistic Swimming"),
            "BallroomDance": qsTr("Ballroom Dance"),
            "Bmx": qsTr("BMX"),
            "BoardGame": qsTr("Board Game"),
            "Bocce": qsTr("Bocce"),
            "Breaking": qsTr("Breaking"),
            "Bridge": qsTr("Bridge"),
            "CardioCombat": qsTr("Cardio Combat"),
            "Checkers": qsTr("Checkers"),
            "Chess": qsTr("Chess"),
            "DragonBoat": qsTr("Dragon Boat"),
            "Esports": qsTr("Esports"),
            "Finswimming": qsTr("Finswimming"),
            "Flowriding": qsTr("Flowriding"),
            "FolkDance": qsTr("Folk Dance"),
            "Frisbee": qsTr("Frisbee"),
            "Futsal": qsTr("Futsal"),
            "HackySack": qsTr("Hacky Sack"),
            "HipHop": qsTr("Hip Hop"),
            "HulaHoop": qsTr("Hula Hoop"),
            "IndoorFitness": qsTr("Indoor Fitness"),
            "IndoorIceSkating": qsTr("Indoor Ice Skating"),
            "JaiAlai": qsTr("Jai Alai"),
            "Judo": qsTr("Judo"),
            "Jujitsu": qsTr("Jujitsu"),
            "MassGymnastics": qsTr("Mass Gymnastics"),
            "ModernDance": qsTr("Modern Dance"),
            "MuayThai": qsTr("Muay Thai"),
            "ParallelBars": qsTr("Parallel Bars"),
            "PoleDance": qsTr("Pole Dance"),
            "RaceWalking": qsTr("Race Walking"),
            "Shuffleboard": qsTr("Shuffleboard"),
            "Snorkeling": qsTr("Snorkeling"),
            "SomatosensoryGame": qsTr("Somatosensory Game"),
            "Spinning": qsTr("Spinning"),
            "SquareDance": qsTr("Square Dance"),
            "StairClimber": qsTr("Stair Climber"),
            "Stepper": qsTr("Stepper"),
            "Stretching": qsTr("Stretching"),
            "TableFootball": qsTr("Table Football"),
            "TugOfWar": qsTr("Tug Of War"),
            "WallBall": qsTr("Wall Ball"),
            "WaterPolo": qsTr("Water Polo"),
            "Weiqi": qsTr("Weiqi"),
            "FreeSparring": qsTr("Free Sparring"),
            "BodyCombat": qsTr("Body Combat"),
            "PlazaDancing": qsTr("Plaza Dancing"),
            "LaserTag": qsTr("Laser Tag"),
            "ObstacleRace": qsTr("Obstacle Race"),
            "BilliardPool": qsTr("Billiard Pool"),
            "Canoeing": qsTr("Canoeing"),
            "WaterScooter": qsTr("Water Scooter"),
            "Bobsleigh": qsTr("Bobsleigh"),
            "Sledding": qsTr("Sledding"),
            "Biathlon": qsTr("Biathlon"),
            "BungeeJumping": qsTr("Bungee Jumping"),
            "Orienteering": qsTr("Orienteering"),
            "Trekking": qsTr("Trekking"),
            "TrailRun": qsTr("Trail Run"),
            "UpperBody": qsTr("Upper Body"),
            "LowerBody": qsTr("Lower Body"),
            "Barbell": qsTr("Barbell"),
            "Triathlon": qsTr("Triathlon"),
            "OtherWaterSports": qsTr("Other Water Sports"),
            "OtherWinterSports": qsTr("Other Winter Sports"),
            "Powerboating": qsTr("Powerboating"),
            "Diving": qsTr("Diving"),
            "Atv": qsTr("ATV"),
            "Paragliding": qsTr("Paragliding"),
            "Weightlifting": qsTr("Weightlifting"),
            "Deadlift": qsTr("Deadlift"),
            "Burpee": qsTr("Burpee"),
            "Abs": qsTr("Abs"),
            "Back": qsTr("Back"),
            "StepAerobics": qsTr("Step Aerobics"),
            "Equestrian": qsTr("Equestrian"),
            "Athletics": qsTr("Athletics")
        };
        if (translations[sportkind] !== undefined)
            return translations[sportkind];

        return sportkind;
    }

    title: qsTr("Sports Activities")
    model: SportsModel
    Component.onCompleted: {
        SportsModel.update();
    }
    onPageStatusActive: {
        pushAttached(Qt.resolvedUrl("BatteryPage.qml"));
    }

    IconPL {
        id: sharedIconTime

        iconName: styler.iconClock
        visible: false
    }

    Connections {
        target: DaemonInterfaceInstance
        onOperationRunningChanged: {
            SportsModel.update();
        }
    }

    pageMenu: PageMenuPL {
        PageMenuItemPL {
            iconSource: styler.iconDownloadData !== undefined ? styler.iconDownloadData : ""
            text: qsTr("Download Next Activity")
            onClicked: DaemonInterfaceInstance.downloadSportsData()
            enabled: DaemonInterfaceInstance.connectionState === "authenticated"
        }

    }

    delegate: ListItemPL {
        id: listItem

        contentHeight: styler.themeItemSizeSmall + (styler.themePaddingMedium * 2)
        anchors.left: parent.left
        anchors.right: parent.right
        width: parent.width
        onClicked: {
            var sportpage = app.pages.push(Qt.resolvedUrl("SportPage.qml"), {
                "activitytitle": translateSportKind(kindstring) + " - " + Qt.formatDateTime(startdate, "yyyy/MM/dd"),
                "date": Qt.formatDateTime(startdate, "yyyy/MM/dd"),
                "location": [baselatitude, baselongitude, basealtitude],
                "starttime": Qt.formatDateTime(startdate, "hh:mm:ss"),
                "duration": durationLabel.text,
                "times": timesLabel.text,
                "kindstring": kindstring,
                "tcx": SportsModel.gpx(id)
            });
            SportsMeta.update(id);
            sportpage.update();
        }

        Row {
            id: listItemRow

            width: parent.width - (2 * styler.themePaddingLarge)
            spacing: styler.themePaddingLarge * 2
            anchors.left: parent.left
            anchors.leftMargin: styler.themePaddingLarge * 2
            anchors.right: parent.right
            anchors.rightMargin: styler.themePaddingLarge * 2

            Loader {
                id: workoutImage

                anchors.top: parent.top
                anchors.topMargin: styler.themePaddingMedium
                width: styler.themeItemSizeSmall
                height: width

                sourceComponent: IconPL {
                    iconSource: styler.customIconPrefix + "icon-m-" + kindstring.toLowerCase() + styler.customIconSuffix
                    width: styler.themeItemSizeSmall
                    height: width
                }

            }

            Column {
                id: leftColumn

                anchors.top: parent.top
                anchors.topMargin: styler.themePaddingMedium
                width: (parent.width - workoutImage.width - (styler.themePaddingLarge * 4)) * 0.5
                spacing: styler.themePaddingSmall

                LabelPL {
                    id: nameLabel

                    text: translateSportKind(kindstring)
                }

                LabelPL {
                    id: dateLabel

                    text: Qt.formatDate(startdate, "ddd") + " " + startdate.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
                }

            }

            Column {
                id: rightColumn

                anchors.top: parent.top
                anchors.topMargin: styler.themePaddingMedium
                width: leftColumn.width
                spacing: styler.themePaddingSmall

                Row {
                    anchors.right: parent.right
                    spacing: styler.themePaddingSmall

                    IconPL {
                        id: durationImage

                        height: durationLabel.height
                        width: height
                        asynchronous: true
                        source: sharedIconTime.source
                    }

                    LabelPL {
                        id: durationLabel

                        text: fncCovertSecondsToString((enddate - startdate) / 1000)
                        horizontalAlignment: Text.AlignRight
                    }

                }

                LabelPL {
                    id: timesLabel

                    anchors.right: parent.right
                    text: startdate.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + " ⟶ " + enddate.toLocaleTimeString(Qt.locale(), Locale.ShortFormat)
                    horizontalAlignment: Text.AlignRight
                }

            }

        }

        menu: ContextMenuPL {
            id: contextMenu

            ContextMenuItemPL {
                iconName: styler.iconDelete
                text: qsTr("Remove")
                onClicked: {
                    SportsModel.deleteRecord(id);
                }
            }

        }

    }

}
