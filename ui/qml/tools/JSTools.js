/*
 * Copyright (C) 2017 Jens Drescher, Germany
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

.pragma library

//gives back random int. min is inclusive and max is exclusive.
function fncGetRandomInt(min, max)
{
    min = Math.ceil(min);
    max = Math.floor(max);
    return Math.floor(Math.random() * (max - min)) + min;
}

function fncPadZeros(number, size)
{
  number = number.toString();
  while (number.length < size) number = "0" + number;
  return number;
}


var arrayDataPoints =
[
    //{ heartrate: 140, elevation: 354.34, distance: 232 }
]

var trackPointsAt = [];
var trackPausePointsTemporary = [];


function fncAddDataPoint(heartrate,elevation,distance,time,unixtime,speed,pace,pacevalue,paceimp,duration)
{
    var iPosition = arrayDataPoints.length;

    arrayDataPoints[iPosition] = new Object();
    arrayDataPoints[iPosition]["heartrate"] = heartrate;
    arrayDataPoints[iPosition]["elevation"] = elevation;
    arrayDataPoints[iPosition]["distance"] = distance;
    arrayDataPoints[iPosition]["time"] = time;
    arrayDataPoints[iPosition]["unixtime"] = unixtime;
    arrayDataPoints[iPosition]["speed"] = speed;
    arrayDataPoints[iPosition]["pace"] = pace;
    arrayDataPoints[iPosition]["pacevalue"] = pacevalue;
    arrayDataPoints[iPosition]["paceimp"] = paceimp;
    arrayDataPoints[iPosition]["duration"] = duration;
}

function fncConvertDistanceToImperial(iKilometers)
{
    return iKilometers * 0.621371192237;
}

function fncConvertelevationToImperial(iMeters)
{
    return iMeters * 3.28084;
}

function fncConvertPacetoImperial(iPace)
{
    return iPace * 1.609344;
}

function fncConvertSpeedToImperial(iSpeed)
{
    return iSpeed * 0.621371192237;
}


//*************** Pebble functions *****************

var arrayPebbleValueTypes =
[
    { index: 0, fieldID: 0, fieldIDCoverPage: 0, value:  "", header: qsTr("Empty"), unit: "", imperialUnit: "" },
    { index: 1, fieldID: 0, fieldIDCoverPage: 0, value: "0", header: qsTr("Heartrate"), unit: "bpm", imperialUnit: "bpm" },
    { index: 2, fieldID: 0, fieldIDCoverPage: 0, value: "0", header: qsTr("Heartrate") + "∅", unit: "bpm", imperialUnit: "bpm" },
    { index: 3, fieldID: 3, fieldIDCoverPage: 3, value: "0", header: qsTr("Pace"), unit: "min/km", imperialUnit: "min/mi" },
    { index: 4, fieldID: 0, fieldIDCoverPage: 0, value: "0", header: qsTr("Pace") + "∅", unit: "min/km", imperialUnit: "min/mi" },
    { index: 5, fieldID: 0, fieldIDCoverPage: 0, value: "0", header: qsTr("Speed"), unit: "km/h", imperialUnit: "mi/h" },
    { index: 6, fieldID: 0, fieldIDCoverPage: 0, value: "0", header: qsTr("Speed") + "∅", unit: "km/h", imperialUnit: "mi/h" },
    { index: 7, fieldID: 0, fieldIDCoverPage: 0, value: "0", header: qsTr("Altitude"), unit: "m", imperialUnit: "ft" },
    { index: 8, fieldID: 2, fieldIDCoverPage: 2, value: "0", header: qsTr("Distance"), unit: "km", imperialUnit: "mi" },
    { index: 9, fieldID: 0, fieldIDCoverPage: 0, value: "0", valueCoverPage: "0", header: qsTr("Pause"), unit: "", imperialUnit: "" },
    { index: 10, fieldID: 1, fieldIDCoverPage: 1, value: "0", valueCoverPage: "0", header: qsTr("Duration"), unit: "", imperialUnit: "" }
]

//Create lookup table for pebble value fields.
//This is a helper table to easier access the main table.
var arrayLookupPebbleValueTypesByFieldID = {};
fncGenerateHelperArray();

function fncGenerateHelperArray()
{
    for (var i = 0; i < arrayPebbleValueTypes.length; i++)
    {
        arrayLookupPebbleValueTypesByFieldID[arrayPebbleValueTypes[i].fieldID] = arrayPebbleValueTypes[i];
    }
}


function fncConvertSaveStringToArray(sSaveString)
{
    //"10,8,3"

    if (sSaveString === undefined || sSaveString === "")
        return;

    var arValueTypes = sSaveString.split(",");

    if (arValueTypes.length !== 3)    //This is the amount pebble fields
        return;

    arValueTypes[0] = parseInt(arValueTypes[0]);
    arValueTypes[1] = parseInt(arValueTypes[1]);
    arValueTypes[2] = parseInt(arValueTypes[2]);

    //Go through value types
    for (var i = 0; i < arrayPebbleValueTypes.length; i++)
    {
        if (i === arValueTypes[0])
            arrayPebbleValueTypes[i].fieldID = 1;
        else if (i === arValueTypes[1])
            arrayPebbleValueTypes[i].fieldID = 2;
        else if (i === arValueTypes[2])
            arrayPebbleValueTypes[i].fieldID = 3;
        else
            arrayPebbleValueTypes[i].fieldID = 0;
    }

    fncGenerateHelperArray();
}

function fncConvertArrayToSaveString()
{
    //"10,8,3"

    var sSaveString = "";

    sSaveString = arrayLookupPebbleValueTypesByFieldID[1].index.toString();
    sSaveString = sSaveString + arrayLookupPebbleValueTypesByFieldID[2].index.toString();
    sSaveString = sSaveString + arrayLookupPebbleValueTypesByFieldID[3].index.toString();

    return sSaveString;
}

//*************** CoverPage functions *****************

//Create lookup table for cover page value fields.
//This is a helper table to easier access the main table.
var arrayLookupCoverPageValueTypesByFieldID = {};
fncGenerateHelperArrayCoverPage();

function fncGenerateHelperArrayCoverPage()
{
    for (var i = 0; i < arrayPebbleValueTypes.length; i++)
    {
        arrayLookupCoverPageValueTypesByFieldID[arrayPebbleValueTypes[i].fieldIDCoverPage] = arrayPebbleValueTypes[i];
    }
}

function fncConvertSaveStringToArrayCoverPage(sSaveString)
{
    //"10,8,3"

    if (sSaveString === undefined || sSaveString === "")
        return;

    var arValueTypes = sSaveString.split(",");

    if (arValueTypes.length !== 3)    //This is the amount pebble fields
        return;

    arValueTypes[0] = parseInt(arValueTypes[0]);
    arValueTypes[1] = parseInt(arValueTypes[1]);
    arValueTypes[2] = parseInt(arValueTypes[2]);

    //Go through value types
    for (var i = 0; i < arrayPebbleValueTypes.length; i++)
    {
        if (i === arValueTypes[0])
            arrayPebbleValueTypes[i].fieldIDCoverPage = 1;
        else if (i === arValueTypes[1])
            arrayPebbleValueTypes[i].fieldIDCoverPage = 2;
        else if (i === arValueTypes[2])
            arrayPebbleValueTypes[i].fieldIDCoverPage = 3;
        else
            arrayPebbleValueTypes[i].fieldIDCoverPage = 0;
    }

    fncGenerateHelperArrayCoverPage();
}

function fncConvertArrayToSaveStringCoverPage()
{
    //"10,8,3"

    var sSaveString = "";

    sSaveString = arrayLookupCoverPageValueTypesByFieldID[1].index.toString();
    sSaveString = sSaveString + arrayLookupCoverPageValueTypesByFieldID[2].index.toString();
    sSaveString = sSaveString + arrayLookupCoverPageValueTypesByFieldID[3].index.toString();

    return sSaveString;
}

//*************** Strava functions *****************



var arrayStravaWorkoutTypes =
[
    { name: "running", stravaType: "Run" },
    { name: "cycling", stravaType: "Ride" },
    { name: "mountainBiking", stravaType: "Ride" },
    { name: "walking", stravaType: "Walk" },
    { name: "inlineSkating", stravaType: "InlineSkate" },
    { name: "skiing", stravaType: "AlpineSki" },
    { name: "hiking", stravaType: "Hike" },
    { name: "indoor cycling", stravaType: "VirtualRide" },
    { name: "treadmill", stravaType: "VirtualRun" }
]

function toStravaType(t)
{
    var ret = "";
    for (var i = 0; i < arrayStravaWorkoutTypes.length; i++)
    {
        if (arrayStravaWorkoutTypes[i].name === t.toLowerCase()) {
            ret = arrayStravaWorkoutTypes[i].stravaType;
            break;
        }
    }
    return ret;
}


function fromStravaType(t)
{
    console.log("looking for type ", t);
    var ret = "";
    for (var i = 0; i < arrayStravaWorkoutTypes.length; i++)
    {
        if (arrayStravaWorkoutTypes[i].stravaType === t.toLowerCase()) {
            ret = arrayStravaWorkoutTypes[i].name;
            break;
        }
    }
    return ret;
}

function stravaGet(xmlhttp, url, token, onready)
{
    console.log("Loading from ", url);

    xmlhttp.open("GET", url);
    xmlhttp.setRequestHeader('Accept-Encoding', 'text');
    xmlhttp.setRequestHeader('Connection', 'keep-alive');
    xmlhttp.setRequestHeader('Pragma', 'no-cache');
    xmlhttp.setRequestHeader('Content-Type', 'application/json');
    xmlhttp.setRequestHeader('Accept', 'application/json, text/plain, */*');
    xmlhttp.setRequestHeader('Cache-Control', 'no-cache');
    xmlhttp.setRequestHeader('Authorization', "Bearer " + token);

    xmlhttp.onreadystatechange=onready;

    xmlhttp.send();
}

function fncCovertMinutesToString(min)
{
    var iHours = Math.floor(min / 3600);
    var iMinutes = Math.floor((min - iHours * 3600) / 60);
    var iSeconds = Math.floor(min - (iHours * 3600) - (iMinutes * 60));

    return (iHours > 0 ? iHours + "h " : "") + (iMinutes > 0 ? iMinutes + "m " : "") + iSeconds + "s";
}

//*************** Voice output functions *****************

var arrayVoiceValueTypes =
[
    { index: 0, fieldID_Duration: 0, fieldID_Distance: 0, value: "", header: qsTr("Empty"), headline: "", unit: "", imperialUnit: "" },
    { index: 1, fieldID_Duration: 0, fieldID_Distance: 0, value: "0", header: qsTr("Heartrate"), headline: "heartrate", unit: "bpm", imperialUnit: "bpm" },
    { index: 2, fieldID_Duration: 0, fieldID_Distance: 0, value: "0", header: qsTr("Heartrate") + "∅", headline: "heartrateavg", unit: "bpm", imperialUnit: "bpm" },
    { index: 3, fieldID_Duration: 4, fieldID_Distance: 3, value: "0", header: qsTr("Pace"), headline: "pace", unit: "minkm", imperialUnit: "minmi" },
    { index: 4, fieldID_Duration: 0, fieldID_Distance: 0, value: "0", header: qsTr("Pace") + "∅", headline: "paceavg", unit: "minkm", imperialUnit: "minmi" },
    { index: 5, fieldID_Duration: 3, fieldID_Distance: 4, value: "0", header: qsTr("Speed"), headline: "speed", unit: "kmh", imperialUnit: "mih" },
    { index: 6, fieldID_Duration: 0, fieldID_Distance: 0, value: "0", header: qsTr("Speed") + "∅", headline: "speedavg", unit: "kmh", imperialUnit: "mih" },
    { index: 7, fieldID_Duration: 0, fieldID_Distance: 0, value: "0", header: qsTr("Altitude"), headline: "altitude", unit: "m", imperialUnit: "ft" },
    { index: 8, fieldID_Duration: 2, fieldID_Distance: 2, value: "0", header: qsTr("Distance"), headline: "distance", unit: "km", imperialUnit: "mi" },
    { index: 9, fieldID_Duration: 1, fieldID_Distance: 1, value: "0", header: qsTr("Duration"), headline: "duration", unit: "duration", imperialUnit: "duration" }
]

//Create lookup table. This is a helper table to easier access the main table.
var arrayLookupVoiceValueTypesByFieldIDDistance = {};
fncGenerateHelperArrayFieldIDDistance();
function fncGenerateHelperArrayFieldIDDistance()
{
    for (var i = 0; i < arrayVoiceValueTypes.length; i++)
    {
        arrayLookupVoiceValueTypesByFieldIDDistance[arrayVoiceValueTypes[i].fieldID_Distance] = arrayVoiceValueTypes[i];
    }
}

var arrayLookupVoiceValueTypesByFieldIDDuration = {};
fncGenerateHelperArrayFieldIDDuration();
function fncGenerateHelperArrayFieldIDDuration()
{
    for (var i = 0; i < arrayVoiceValueTypes.length; i++)
    {
        arrayLookupVoiceValueTypesByFieldIDDuration[arrayVoiceValueTypes[i].fieldID_Duration] = arrayVoiceValueTypes[i];
    }
}

function fncConvertSaveStringToArrayCyclicVoiceDistance(sSaveString)
{
    //"8,7,2,4"

    if (sSaveString === undefined || sSaveString === "")
        return;

    var arValueTypes = sSaveString.split(",");

    if (arValueTypes.length !== 4)    //This is the amount of voice cycle fields
        return;

    arValueTypes[0] = parseInt(arValueTypes[0]);
    arValueTypes[1] = parseInt(arValueTypes[1]);
    arValueTypes[2] = parseInt(arValueTypes[2]);
    arValueTypes[3] = parseInt(arValueTypes[3]);

    //Go through value types
    for (var i = 1; i < arrayVoiceValueTypes.length; i++)
    {
        if (i === arValueTypes[0])
            arrayVoiceValueTypes[i].fieldID_Distance = 1;
        else if (i === arValueTypes[1])
            arrayVoiceValueTypes[i].fieldID_Distance = 2;
        else if (i === arValueTypes[2])
            arrayVoiceValueTypes[i].fieldID_Distance = 3;
        else if (i === arValueTypes[3])
            arrayVoiceValueTypes[i].fieldID_Distance = 4;
        else
            arrayVoiceValueTypes[i].fieldID_Distance = 0;
    }   

    fncGenerateHelperArrayFieldIDDistance();
}

function fncConvertArrayToSaveStringCyclicVoiceDistance()
{
    //"8,7,2,4"

    var sSaveString = "";

    sSaveString = arrayLookupVoiceValueTypesByFieldIDDistance[1].index.toString();
    sSaveString = sSaveString + arrayLookupVoiceValueTypesByFieldIDDistance[2].index.toString();
    sSaveString = sSaveString + arrayLookupVoiceValueTypesByFieldIDDistance[3].index.toString();
    sSaveString = sSaveString + arrayLookupVoiceValueTypesByFieldIDDistance[4].index.toString();

    return sSaveString;
}

function fncConvertSaveStringToArrayCyclicVoiceDuration(sSaveString)
{
    //"8,7,4,2"

    if (sSaveString === undefined || sSaveString === "")
        return;

    var arValueTypes = sSaveString.split(",");

    if (arValueTypes.length !== 4)    //This is the amount of voice cycle fields
        return;

    arValueTypes[0] = parseInt(arValueTypes[0]);
    arValueTypes[1] = parseInt(arValueTypes[1]);
    arValueTypes[2] = parseInt(arValueTypes[2]);
    arValueTypes[3] = parseInt(arValueTypes[3]);

    //Go through value types
    for (var i = 1; i < arrayVoiceValueTypes.length; i++)
    {
        if (i === arValueTypes[0])
            arrayVoiceValueTypes[i].fieldID_Duration = 1;
        else if (i === arValueTypes[1])
            arrayVoiceValueTypes[i].fieldID_Duration = 2;
        else if (i === arValueTypes[2])
            arrayVoiceValueTypes[i].fieldID_Duration = 3;
        else if (i === arValueTypes[3])
            arrayVoiceValueTypes[i].fieldID_Duration = 4;
        else
            arrayVoiceValueTypes[i].fieldID_Duration = 0;
    }

    fncGenerateHelperArrayFieldIDDuration();
}

function fncConvertArrayToSaveStringCyclicVoiceDuration()
{
    //"8,7,2,4"

    var sSaveString = "";

    sSaveString = arrayLookupVoiceValueTypesByFieldIDDuration[1].index.toString();
    sSaveString = sSaveString + arrayLookupVoiceValueTypesByFieldIDDuration[2].index.toString();
    sSaveString = sSaveString + arrayLookupVoiceValueTypesByFieldIDDuration[3].index.toString();
    sSaveString = sSaveString + arrayLookupVoiceValueTypesByFieldIDDuration[4].index.toString();

    return sSaveString;
}

function fncPlayCyclicVoiceAnnouncement(bMetric, iVoiceLanguage, bDistance, bPlayHeadline)
{
    var arraySoundArray = [];
    var arrayTempArray = [];    
    var arrayLookUpArray = [];

    if (bDistance)
        arrayLookUpArray = arrayLookupVoiceValueTypesByFieldIDDistance;
    else
        arrayLookUpArray = arrayLookupVoiceValueTypesByFieldIDDuration;

    //We have a maximum of 4 voice messages to play
    for (var i = 1; i < 5; i++)
    {
        console.log("arrayLookUpArray[" + i.toString() + "].index: " + arrayLookUpArray[i].index.toString());
        console.log("arrayLookUpArray[" + i.toString() + "].fieldID_Duration: " + arrayLookUpArray[i].fieldID_Duration.toString());
        console.log("arrayLookUpArray[" + i.toString() + "] === undefined: " + (arrayLookUpArray[i] === undefined).toString());

        //Check if index exists
        if (arrayLookUpArray[i] === undefined)
            continue;

        //Check if ths is an empty entry
        if ((bDistance && arrayLookUpArray[i].fieldID_Distance === 0) || (!bDistance && arrayLookUpArray[i].fieldID_Duration === 0))
            continue;

        //Get value
        var iNumber = (arrayLookUpArray[i].value === "0") ? 0 : arrayLookUpArray[i].value;
        //Get unit
        var sUnit = (bMetric) ? arrayLookUpArray[i].unit : arrayLookUpArray[i].imperialUnit;
        //Get headline
        var sHeadline = arrayLookUpArray[i].headline;

        //Is it duration? Then we need a special treatment
        if (sHeadline === "duration")
        {
            if (iNumber === 0)
                continue;

            //console.log("Duration playing iNumber: " + iNumber.toString());

            //value is something like >00h 00m 00s<
            //Separate the three values
            var sSplitArray = iNumber.split(" ");

            //TODO/DEBUG: SplitArray can consist of only 1 entry!!!
            //console.log("sSplitArray.length: " + sSplitArray.length.toString());

            var iHour = 0;
            var iMinute = 0;
            var iSecond = 0;

            //SplitArray should have 3 entries (h,m,s)
            if (sSplitArray.length === 1) //only seconds
            {
                iSecond = parseInt(sSplitArray[0]);
            }
            else if (sSplitArray.length === 2) //seconds and minutes
            {
                iMinute = parseInt(sSplitArray[0]);
                iSecond = parseInt(sSplitArray[1]);
            }
            else if (sSplitArray.length === 3) //seconds and minutes and hours
            {
                iHour = parseInt(sSplitArray[0]);
                iMinute = parseInt(sSplitArray[1]);
                iSecond = parseInt(sSplitArray[2]);
            }
            else
                continue;

            var sUnitHour = (iHour === 1) ? "hour" : "hours";
            var sUnitMinute = (iMinute === 1) ? "minute" : "minutes";
            var sUnitSecond = (iSecond === 1) ? "second" : "seconds";

            var bPlayHour = (iHour !== 0);
            var bPlayMinute = (iMinute !== 0);
            var bPlaySecond = (iSecond !== 0);

            arrayTempArray = [];
            if (bPlayHour)
                arrayTempArray = fncGenerateSoundArray(iHour, sUnitHour, bPlayHeadline ? sHeadline : "", iVoiceLanguage);

            if (arrayTempArray !== undefined)
            {
                for (var j = 0; j < arrayTempArray.length; j++)
                {
                    arraySoundArray.push(arrayTempArray[j]);
                }
            }

            arrayTempArray = [];
            if (bPlayMinute)
            {
                if (bPlayHour)
                    arrayTempArray = fncGenerateSoundArray(iMinute, sUnitMinute, "", iVoiceLanguage);
                else
                    arrayTempArray = fncGenerateSoundArray(iMinute, sUnitMinute, bPlayHeadline ? sHeadline : "", iVoiceLanguage);
            }

            if (arrayTempArray !== undefined)
            {
                for (j = 0; j < arrayTempArray.length; j++)
                {
                    arraySoundArray.push(arrayTempArray[j]);
                }
            }

            arrayTempArray = [];
            if (bPlaySecond)
            {
                if (bPlayHour || bPlayMinute)
                    arrayTempArray = fncGenerateSoundArray(iSecond, sUnitSecond, "", iVoiceLanguage);
                else
                    arrayTempArray = fncGenerateSoundArray(iSecond, sUnitSecond, bPlayHeadline ? sHeadline : "", iVoiceLanguage);
            }

            if (arrayTempArray !== undefined)
            {
                for (var j = 0; j < arrayTempArray.length; j++)
                {
                    arraySoundArray.push(arrayTempArray[j]);
                }
            }
        }
        else if (sHeadline === "pace")
        {
            if (iNumber === 0)
                continue;

            //value is something like >00:00<
            //Separate the three values
            var sSplitArray = iNumber.split(":");
            //SplitArray must have 2 entries (m:s)
            if (sSplitArray.length !== 2)
                continue;
            var iMinute = parseInt(sSplitArray[0]);
            var iSecond = parseInt(sSplitArray[1]);

            //if one of the numbers s too high
            if (iMinute > 699 || iHour > 699)
                continue;

            var sUnitMinute = (iMinute === 1) ? "minute" : "minutes";
            var sUnitSecond = (iSecond === 1) ? "second" : "seconds";

            arrayTempArray = fncGenerateSoundArray(iMinute, sUnitMinute, bPlayHeadline ? sHeadline : "", iVoiceLanguage);
            if (arrayTempArray !== undefined)
            {
                for (j = 0; j < arrayTempArray.length; j++)
                {
                    arraySoundArray.push(arrayTempArray[j]);
                }
            }

            arrayTempArray = [];
            arrayTempArray = fncGenerateSoundArray(iSecond, sUnitSecond, "", iVoiceLanguage);
            if (arrayTempArray !== undefined)
            {
                for (var j = 0; j < arrayTempArray.length; j++)
                {
                    arraySoundArray.push(arrayTempArray[j]);
                }
            }

            //lastly we need to add the unit
            arrayTempArray = [];
            arrayTempArray = fncGenerateSoundArray("", sUnit, "", iVoiceLanguage);
            if (arrayTempArray !== undefined)
            {
                for (var j = 0; j < arrayTempArray.length; j++)
                {
                    arraySoundArray.push(arrayTempArray[j]);
                }
            }
        }
        else
        {
            //Covert value and unit to audio file array
            arrayTempArray = fncGenerateSoundArray(iNumber, sUnit, bPlayHeadline ? sHeadline : "", iVoiceLanguage);

            if (arrayTempArray !== undefined)
            {
                for (var j = 0; j < arrayTempArray.length; j++)
                {
                    arraySoundArray.push(arrayTempArray[j]);
                }
            }
        }

        //ToDo: eventually put in a pause here
    }

    return arraySoundArray;
}

function fncGenerateSoundArray(number, sUnit, sHeadline, iVoiceLanguage)
{
    //So in german we have a little problem here: the 1.
    //This number can be as "eins" or "eine" or "ein".
    //"eins" (1_de_male.wav) : if number is a float we must use this for both left and right of the point.
    //"eine" (1e_de_male.wav): if unit is time (hour, minute or second) then we use this one. Number should be integer then.
    //"ein"  (1n_de_male.wav) : if number is an integer and we don't have a time unit, then use this one.

    //console.log("number: " + number);
    //console.log("sUnit: " + sUnit);
    //console.log("sHeadline: " + sHeadline);
    //console.log("iVoiceLanguage: " + iVoiceLanguage);

    var arraySoundArray = [];
    var sNumberToPlay = "";
    var sVoiceLanguage = "_en_male.wav";
    var bOneIsSingular = false;

    //check voice language and generate last part of audio filename
    if (iVoiceLanguage === 0)        //english male
        sVoiceLanguage = "_en_male.wav";
    else if (iVoiceLanguage === 1)   //german male
        sVoiceLanguage = "_de_male.wav";   

    //add the headline
    if (sHeadline !== "")
    {
        arraySoundArray.push("headers/" + sHeadline + sVoiceLanguage);
    }

    if (number === "")  //This is for when number should not be played
    {

    }
    else if (isInteger(number))		//Check if it's an integer
    {
        //Check if we need a singular voice file for the unit. Plural is default.
        if (number === 1)
            bOneIsSingular = true;

        //console.log("Number is INT");

        //Check limits
        if (number > 699 || number < 0)
            return;

        if (number >= 100)
        {
            var sHundreds = parseInt(number.toString().substr(0,1) + "00");

            //Cut off hundreds
            number = parseInt(number.toString().substr(1));

            arraySoundArray.push("numbers/" + sHundreds.toString() + sVoiceLanguage);
        }

        if (number === 1 && (sUnit === "hour" || sUnit === "minute" || sUnit === "second"))
            arraySoundArray.push("numbers/1e" + sVoiceLanguage);
        else if (number === 1)
            arraySoundArray.push("numbers/1n" + sVoiceLanguage);
        else
            arraySoundArray.push("numbers/" + number.toString() + sVoiceLanguage);
    }
    else //should be a float
    {
        //console.log("Number is FLOAT");

        var sFloatArray = number.toString().split(".");

        if (typeof sFloatArray === 'undefined' || sFloatArray.length !== 2)
            sFloatArray = number.toString().split(",");

        if (typeof sFloatArray === 'undefined' || sFloatArray.length !== 2)
            return;       

        //push first place. First check for size over hundred.
        if (parseInt(sFloatArray[0]) >= 100)
        {
            var sHundreds = parseInt(sFloatArray[0].substr(0,1) + "00");

            //Cut off hundreds
            sFloatArray[0] = parseInt(sFloatArray[0].substr(1));

            arraySoundArray.push("numbers/" + sHundreds.toString() + sVoiceLanguage);
        }
        arraySoundArray.push("numbers/" + sFloatArray[0] + sVoiceLanguage);

        //we only use one decimal point, check that.
        if (sFloatArray[1].length > 1)
            sFloatArray[1] = sFloatArray[1].substr(0,1);

        //push decimal place
        arraySoundArray.push("numbers/" + "dot_" + sFloatArray[1] + sVoiceLanguage);
    }   

    //add the unit
    if (sUnit !== "")
    {    
        if (bOneIsSingular && (sUnit === "km" || sUnit === "mi") || sUnit === "m")
            arraySoundArray.push("units/" + sUnit + "_singular" + sVoiceLanguage);    //this is for singular unit e.g. 1 kilometer
        else
            arraySoundArray.push("units/" + sUnit + sVoiceLanguage);                 //This is for plural unit e.g. 2 kilometers
    }

    return arraySoundArray;
}

//Warning: this is false for 0.0
function isFloat(n)
{
    return n === +n && n !== (n|0);
}

function isInteger(n)
{
    return n === +n && n === (n|0);
}
