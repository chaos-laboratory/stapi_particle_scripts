#include <Wire.h> //Import Arduino Wire Library
#define T6713_Address 0x15 //T6713 i2C Address
/*Sensorthings API integration with Particle Device
The sensorthings API data model is used (https://developers.sensorup.com/docs/#introduction)
!!! indicates user input required
/*=============================================================================================================================================
!!! MANDATORY PARAMETERS
=============================================================================================================================================*/
String thingDesc = ""; //DESCRIBE THE DEPLOYMENT
int sample_rate = 10 * 1000; // how often you want device to publish where the number in front of a thousand is the seconds you want
/*=============================================================================================================================================
OPTIONAL PARAMETERS 
(this parameters only works when it is the first time registration of the device)
(if the device is already registered, these options are ignored)
=============================================================================================================================================*/
//Description of the location
String foiName = "na"; //THE LOCATION NAME
String foiDesc = "na";
//Define the geometry of the location. Geometry types from geojson is accepted. Refer to https://tools.ietf.org/html/rfc7946 for geometry types.
String locType = "Point";
String enType = "application/vnd.geo+json";
//Define the location of the thing. If location is not impt, use [0,0,0] the null island position.
float coordx = 0.0; //lon/x
float coordy = 0.0; //lat/y
float coordz = 0.0; //elv/z
/*=============================================================================================================================================
END
=============================================================================================================================================*/
unsigned char data[6];

String pins = "I2C-T6713"; //describe the pins connection on the particle device
const int nDs = 1; // how many sensors are attached to this particle device that is streaming single number result
//Specify the Ids of the sensors that is attached to each datastream
int dsSnrIds[nDs] = {6}; // Visit http://andlchaos300l.princeton.edu:8080/FROST-Server/v1.0/Sensors to look at the sensor catalogue, if you can't find the sensors you need run STAPI04-RegisterSensors
//Specify the Ids of the observation properties that is attached to each datastream
int dsObsPropIds[nDs] = {5};// Visit https://andlchaos300l.princeton.edu:8080/FROST-Server/v1.0/ObservedProperties to look at the observed properties catalogue, if you can't find the observed properties you need run STAPI05-RegisterObservedProperties
//descriptions of the datastream
String dsDescs[nDs] = {"co2"};
String dsObsTypes[nDs] = {"http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"}; // visit http://defs.opengis.net/elda-common/ogc-def/resource?uri=http://www.opengis.net/def/observationType/OGC-OM/2.0/&_format=html and choose one of the observation types
//unit of measurement of the data
String dsUomNames[nDs] = {"parts per million"}; //name of unit of measurement e.g. temperature, daylight
String dsUomSyms[nDs] = {"ppm"}; // the symbols of the unit of measurement e.g. C, lux
String dsUomDefs[nDs] = {"number of particles per million particles"}; //visit "http://www.qudt.org/qudt/owl/1.0.0/unit/Instances.html" to get the definition

int dsId_co2;
int dsIds[nDs] = {0};

int carbonDioxide;

//Specify the app that is flashed on the particle device
String appName = "STAPI_1_T6713";
String dsIdString = String::format(
    "{ \"%s\" : \"%d\"}", 
    "dsId_co2",  dsId_co2);

String thingName;
String deviceId = System.deviceID();
int deviceName = -1;
int thingCnt = -1;
int thingId = -1;
int checked = -1;

int dsId = 0;



void setup() {
    //get the device name
    Particle.subscribe("particle/device/name", getDeviceName);
    Particle.publish("particle/device/name");
    Wire.begin();
    //Set the Datastream Ids as variables
    Particle.variable("datastreamId",dsIdString);
    Particle.variable("appName", appName);
}

void loop() {
    if (thingCnt == -1) {
        /*Check if this particle device is already registered in the database
        a sensorThing query is sent thru the 'getFilter' webhook, 
        we will receive a JSON reply from our database
        the appropriate reaction is coded in the countThing function*/
        while (deviceName == -1) {
            delay(1000);
        }
        Particle.unsubscribe();
        delay(3000);
        Particle.subscribe(System.deviceID() + "/hook-response/getFilter/", countThing, MY_DEVICES);
        String filterCount = String::format("Things?$filter=name eq '%s' and description eq '%s' and properties/uid eq '%s' and properties/pins eq '%s'&$select=id&$count=true&$skip=10000000", thingName.c_str(), thingDesc.c_str(), deviceId.c_str(), pins.c_str());
        String getCount = String::format( "{ \"query\" : \"%s\"}", filterCount.c_str());
        Particle.publish("getFilter", getCount, PRIVATE);
        delay(30000);
    }
    else if (thingCnt==0){
        Particle.unsubscribe();
        //Post the information and trigger the PostThing Webhook.
        //This device has never been registered in the database
        String postThing = String::format(
        "{ \"loctype\" : \"%s\", \"coordx\": \"%f\", \"coordy\": \"%f\", \"coordz\": \"%f\", \"foiname\": \"%s\", \"foidesc\": \"%s\", \"entype\": \"%s\", \"thingdesc\": \"%s\", \"thingname\": \"%s\", \"pins\": \"%s\" }", 
        locType.c_str(), coordx, coordy, coordz, foiName.c_str(), foiDesc.c_str(), enType.c_str(), thingDesc.c_str(), thingName.c_str(), pins.c_str());
        Particle.publish("postThing", postThing, PRIVATE);
        delay(5000);
        //once registered the device get its Id
        Particle.subscribe(System.deviceID() + "/hook-response/getFilter/", getThingId, MY_DEVICES);
        
        String filterId = String::format("Things?$filter=name eq '%s' and description eq '%s' and properties/uid eq '%s' and properties/pins eq '%s'&$select=id", thingName.c_str(), thingDesc.c_str(), deviceId.c_str(), pins.c_str());
        String getId = String::format( "{ \"query\" : \"%s\"}", filterId.c_str());
        Particle.publish("getFilter", getId, PRIVATE);
        delay(1000);
        Particle.publish("REGISTERING", "WAITING FOR THING ID .. .. ", PRIVATE);
        while (thingId == -1) {
            delay(3000);
        }
        
        Particle.unsubscribe();
        for(int ii=0; ii < nDs; ii++ ) {
            String dsName = thingName + "_" + dsDescs[ii];
            String postDs = String::format(
            "{ \"dsname\" : \"%s\", \"dsdesc\": \"%s\", \"obstype\": \"%s\", \"uomname\": \"%s\", \"uomsym\": \"%s\", \"uomdef\": \"%s\", \"thingid\": \"%d\", \"obspropid\": \"%d\", \"sensorid\": \"%d\"}", 
            dsName.c_str(), dsDescs[ii].c_str(), dsObsTypes[ii].c_str(), dsUomNames[ii].c_str(), dsUomSyms[ii].c_str(), dsUomDefs[ii].c_str(), thingId, dsObsPropIds[ii], dsSnrIds[ii]);
            Particle.publish("postDatastream", postDs, PRIVATE);
            delay(5000);
        }
        
        delay(5000);
        Particle.subscribe(System.deviceID() + "/hook-response/getFilter/", checkThingStatus, MY_DEVICES);
        String filterStatus = String::format("Things(%d)?$select=id&$expand=Datastreams($select=id),MultiDatastreams($select=id)", thingId);
        String getStatus = String::format( "{ \"query\" : \"%s\"}", filterStatus.c_str());
        Particle.publish("getFilter", getStatus, PRIVATE);
        delay(1000);
        
        while (checked == -1) {
            delay(1000);
        }
        
        Particle.unsubscribe();
        Particle.subscribe(System.deviceID() + "/hook-response/getFilter/", retrieveThings, MY_DEVICES);
        String filterShow = String::format("Things(%d)?$select=id,name,properties&$expand=Datastreams($select=id,name),MultiDatastreams($select=id,name)", thingId);
        String getShow = String::format( "{ \"query\" : \"%s\"}", filterShow.c_str());
        Particle.publish("getFilter", getShow, PRIVATE);
        delay(1000);
        
        thingCnt=1;
    }
    
    else if (thingCnt == 1) {
        Particle.unsubscribe();
        delay(3000);
        
        Particle.subscribe(System.deviceID() + "/hook-response/getFilter/", getThingId, MY_DEVICES);
        String filterId = String::format("Things?$filter=name eq '%s' and description eq '%s' and properties/uid eq '%s' and properties/pins eq '%s'&$select=id", thingName.c_str(), thingDesc.c_str(), deviceId.c_str(), pins.c_str());
        String getId = String::format( "{ \"query\" : \"%s\"}", filterId.c_str());
        Particle.publish("getFilter", getId, PRIVATE);
        delay(1000);
        Particle.publish("GETTING", "WAITING FOR THING ID .. .. ", PRIVATE);
        while (thingId == -1) {
            delay(3000);
        }
        
        Particle.unsubscribe();
        String dupDsMsg = String::format("POSTING TO THE DATASTREAMS ...", thingCnt);
        Particle.publish("INSTRUCTION", dupDsMsg, PRIVATE);
        delay(5000);
        
        //GET THE DATASTREAM IDS
        Particle.subscribe(System.deviceID() + "/hook-response/getFilter/", getDsId, MY_DEVICES);
        for(int ii=0; ii < nDs; ii++ ) {
            String dsName = thingName + "_" + dsDescs[ii];
            String filterDs = String::format("Datastreams?$filter=name eq '%s' and Things/id eq '%d'&$select=id", dsName.c_str(), thingId);
            String getDs = String::format( "{ \"query\" : \"%s\"}", filterDs.c_str());
            Particle.publish("getFilter", getDs, PRIVATE);
            
            Particle.publish("GETTING", "WAITING FOR DS ID .. .. ", PRIVATE);
            while (dsId == 0) {
                delay(3000);
            }
            dsIds[ii] = dsId;
            dsId = 0;
        }
        
        dsId_co2 = dsIds[0];
        
        dsIdString = String::format(
        "{ \"%s\" : \"%d\"}", 
        "dsId_co2", dsId_co2);
        
        thingCnt=-2;
    }
    
    else if (thingCnt > 1) {
        Particle.unsubscribe();
        
        String dupDsMsg = String::format("%d DUPLICATES HAVE BEEN FOUND IN THE DATABASE", thingCnt);
        Particle.publish("INSTRUCTION", dupDsMsg, PRIVATE);
        delay(1000);
        Particle.publish("INSTRUCTION", "CHANGE THE thingDesc PARAMETER TO REGISTER A NEW DEPLOYMENT OR DELETE THE DUPLICATES", PRIVATE);
        delay(1000);
        
        Particle.subscribe(System.deviceID() + "/hook-response/getFilter/", retrieveThings, MY_DEVICES);
        String filterId = String::format("Things?$filter=properties/uid eq '%s'&$select=id", deviceId.c_str());
        String getId = String::format( "{ \"query\" : \"%s\"}", filterId.c_str());
        Particle.publish("getFilter", getId, PRIVATE);
        delay(1000);
        
        thingCnt=-3;
    }
    
    // THIS IS WHERE YOU INPUT THE SCRIPT TO GET DATA FROM THE SENSOR AND POST DATA TO THE DATABASE 
    else if (thingCnt == -2) {
        Particle.unsubscribe();
        
        //get the current time
        String timeNow = Time.format(Time.now(), TIME_FORMAT_ISO8601_FULL);
        
        //get the reading from the sensor
        carbonDioxide = readT6713();
        
        //post and publish the readings from the first sensor
        String postObsCo2 = String::format(
        "{ \"time\" : \"%s\", \"result\": \"%d\", \"dsId\": \"%d\"}", 
        timeNow.c_str(), carbonDioxide, dsId_co2);
        Particle.publish("postObservation", postObsCo2, PRIVATE);
        delay(sample_rate/3);
        
    }
}

/*=============================================================================================================================================
FUNCTIONS
=============================================================================================================================================*/
void getDeviceName(const char *topic, const char *data) {
    thingName = data;
    deviceName = 1;
}

void countThing(const char *event, const char *data) {
    JSONValue outerObj = JSONValue::parseCopy(data);
    JSONObjectIterator iter(outerObj);
    while(iter.next()) {
        if (iter.name() == "@iot.count") {
            thingCnt = iter.value().toInt();
        }
    }
}

void checkThingStatus(const char *event, const char *data) {
    int dsCnt;
    int mdsCnt;
    
    JSONValue outerObj = JSONValue::parseCopy(data);
    JSONObjectIterator iter(outerObj);
    while(iter.next()) {
        if (iter.name() == "Datastreams"){
            JSONArrayIterator iter2(iter.value());
            dsCnt = iter2.count();
        }
        
        else if (iter.name() == "MultiDatastreams"){
            JSONArrayIterator iter3(iter.value());
            mdsCnt = iter3.count();
        }
    }
    
    if (dsCnt == nDs) {
        checked = 1;
        Particle.publish("REGISTRATION_SUCCESS", "USE THE DETAILS TO POST OBSERVATION IN THE STAPI06_PostObservation", PRIVATE);
        delay(1000);
    }
    
    else {
        checked = 1;
        Particle.publish("REGISTRATION_FAILED", "PLEASE TRY AGAIN OR CHECK THE DATABASE", PRIVATE);
        delay(1000);
    }
    
}

void getThingId(const char *event, const char *data) {
    JSONValue outerObj = JSONValue::parseCopy(data);
    JSONObjectIterator iter(outerObj);
    while(iter.next()) {
        JSONArrayIterator iter2(iter.value());
        for(size_t ii = 0; iter2.next(); ii++) {
            JSONObjectIterator iter3(iter2.value());
            while(iter3.next()){
                if(iter3.name() == "@iot.id"){
                    thingId = iter3.value().toInt();
                }
            }
        }
    }
    
}

void getDsId(const char *event, const char *data) {
    JSONValue outerObj = JSONValue::parseCopy(data);
    JSONObjectIterator iter(outerObj);
    while(iter.next()) {
        JSONArrayIterator iter2(iter.value());
        for(size_t ii = 0; iter2.next(); ii++) {
            JSONObjectIterator iter3(iter2.value());
            while(iter3.next()){
                if(iter3.name() == "@iot.id"){
                    dsId = iter3.value().toInt();
                }
            }
        }
    }
    
}

void retrieveThings(const char *event, const char *data) {
    Particle.publish("DETAILS", data, PRIVATE);
    delay(1000);
}

int readT6713(){
    Wire.beginTransmission(T6713_Address);
    Wire.write(0x04);
    Wire.write(0x13);
    Wire.write(0x8B);
    Wire.write(0x00);
    Wire.write(0x01);
    Wire.endTransmission();
    delay(2000);
    
    Wire.requestFrom(T6713_Address, 4); //Request 4 bytes from the sensor
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
    //ppmValue = data[2] * 256 + data[3];
    int carbonDioxide = ((data[2] & 0x3F ) << 8) | data[3];
    return carbonDioxide;
}
