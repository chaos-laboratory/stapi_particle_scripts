/*Sensorthings API integration with Particle Device
!!! indicates user input required=*/

/*=============================================================================================================================================
!!! FILL IN THESE PARAMETERS !!!
=============================================================================================================================================
The sensorthings API data model is used (https://developers.sensorup.com/docs/#introduction)*/
String thingName = "Arg029"; //the given name pasted on the device 
String thingDesc = "CO2 Sensing in Dillon Gym"; //Must be unique and different from the previous deployment, describe the deployment
String pins = ""; //describe the pins connection on the particle device
//Description of the location
String foiName = "Dillon Gym";
String foiDesc = "";
//Define the geometry of the location. Geometry types from geojson is accepted. Refer to https://tools.ietf.org/html/rfc7946 for geometry types.
String locType = "Point";
String enType = "application/vnd.geo+json";
//Define the location of the thing. If location is not impt, use [0,0,0] the null island position.
float coordx = -74.6585654313478; //lon/x
float coordy = 40.345666918466456; //lat/y
float coordz = 0.0; //elv/z
/*Define the datastream, a datastream is streaming measurements from a sensor
each stream is associated with 1 sensor. There are two types of streams: datastreams and multidatstream 
1) datastream #measurement consists of only a number etc. temperature(C) or 
2) multidatastream #measurement consist of two numbers etc. gps(lon, lat)
Define your datastreams*/
const int nDs = 3; // how many sensors are attached to this particle device that is streaming single number result
//Specify the Ids of the sensors that is attached to each datastream
int dsSnrIds[nDs] = {5,5,5}; // Visit http://chaosbox.princeton.edu:8080/FROST-Server/v1.0/Sensors to look at the sensor catalogue, if you can't find the sensors you need run STAPI04-RegisterSensors
//Specify the Ids of the observation properties that is attached to each datastream
int dsObsPropIds[nDs] = {1,3,5}; //temperature, RH, CO2 // Visit http://chaosbox.princeton.edu:8080/FROST-Server/v1.0/ObservedProperties to look at the observed properties catalogue, if you can't find the observed properties you need run STAPI05-RegisterObservedProperties
//descriptions of the datastream
String dsDescs[nDs] = {"air temp", "relative humidity", "CO2" };
String dsObsTypes[nDs] = {"http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement", "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement", "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"}; // visit http://defs.opengis.net/elda-common/ogc-def/resource?uri=http://www.opengis.net/def/observationType/OGC-OM/2.0/&_format=html and choose one of the observation types
//unit of measurement of the data
String dsUomNames[nDs] = {"degreeCelsius", "percentage", "parts per million"}; //name of unit of measurement e.g. temperature, daylight
String dsUomSyms[nDs] = {"degC", "%", "ppm"}; // the symbols of the unit of measurement e.g. C, lux
String dsUomDefs[nDs] = {"http://www.qudt.org/qudt/owl/1.0.0/unit/Instances.html#DegreeCelsius", "relative humidity", "number of particle in a million particles"}; //visit "http://www.qudt.org/qudt/owl/1.0.0/unit/Instances.html" to get the definition
//define multidatastreams #measurement consist of two numbers etc. gps(lon, lat)
const int nMds = 0; // how many sensors are attached to this particle device that is streaming duo number result
//Specify the Ids of the sensors that is attached to each multidatastream
int mdsSnrIds[nMds] = {};
//Specify the Ids of the observation properties that is attached to each datastream
int mdsObsPropIds[nMds] = {};
//descriptions of the datastream
String mdsDescs[nMds] = {};
String mdsObsTypes[nMds] = {};
//unit of measurement of the data
String mdsUomNames[nMds] = {};
String mdsUomSyms[nMds] = {};
String mdsUomDefs[nMds] = {};
//elements in sensorIds, obsPropIds and dsOrMds must correspond.
/*=============================================================================================================================================
END
=============================================================================================================================================*/
String deviceId = System.deviceID();
int thingCnt = -1;
int thingId = -1;
int thingPropCnt = -1;
int checked = -1;

void setup() {
    //Subscribe to the event stream so we can response in the countThing function
    Particle.subscribe(System.deviceID() + "/hook-response/getFilter/", countThing, MY_DEVICES);
}

void loop() {
    if (thingCnt==-1){
        /*Check if this particle device is already registered in the database
        a sensorThing query is sent thru the 'getFilter' webhook, 
        we will receive a JSON reply from our database
        the appropriate reaction is coded in the countThing function*/
        String filterCount = String::format("Things?$filter=properties/uid eq '%s'&$select=id&$count=true&$skip=10000000", deviceId.c_str());
        String getCount = String::format( "{ \"query\" : \"%s\"}", filterCount.c_str());
        Particle.publish("getFilter", getCount, PRIVATE);
        delay(30000);
    }
    else if (thingCnt==0){
        Particle.publish("INSTRUCTION", "PLEASE USE STAPI02-RegisterDevice FOR REGISTERING NEW DEVICE, THIS DEVICE IS NOT CURRENTLY REGISTERED IN THE DATABASE", PRIVATE);
        delay(1000);
        thingCnt=-2;
    }
    
    else if (thingCnt >= 1) {
        Particle.unsubscribe();
        Particle.subscribe(System.deviceID() + "/hook-response/getFilter/", countDeployment, MY_DEVICES);
        String filterProp = String::format("Things?$filter=name eq '%s' and description eq '%s'&$select=id&$count=true&$skip=1000000", thingName.c_str(), thingDesc.c_str());
        String getProp = String::format( "{ \"query\" : \"%s\"}", filterProp.c_str());
        Particle.publish("getFilter", getProp, PRIVATE);
        delay(1000);
        Particle.publish("REGISTERING", "COUNTING EXISTING DEPLOYMENTS .. .. ", PRIVATE);
        while (thingPropCnt == -1) {
            delay(3000);
        }
        
        Particle.unsubscribe();
        
        if (thingPropCnt == 0) {
            //There are no existing deployment and this is a new deployment 
            //Post the information and trigger the PostThing Webhook.
            String postThing = String::format(
            "{ \"loctype\" : \"%s\", \"coordx\": \"%f\", \"coordy\": \"%f\", \"coordz\": \"%f\", \"foiname\": \"%s\", \"foidesc\": \"%s\", \"entype\": \"%s\", \"thingdesc\": \"%s\", \"thingname\": \"%s\", \"pins\": \"%s\" }", 
            locType.c_str(), coordx, coordy, coordz, foiName.c_str(), foiDesc.c_str(), enType.c_str(), thingDesc.c_str(), thingName.c_str(), pins.c_str());
            Particle.publish("postThing", postThing, PRIVATE);
            delay(10000);
            
            //once registered the device get its Id
            Particle.subscribe(System.deviceID() + "/hook-response/getFilter/", getThingId, MY_DEVICES);
            String filterId = String::format("Things?$filter=name eq '%s'and description eq '%s'&$select=id", thingName.c_str(), thingDesc.c_str());
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
            
            for(int ii=0; ii < nMds; ii++ ) {
                String mdsName = thingName + mdsDescs[ii];
                String postDs = String::format(
                "{ \"dsname\" : \"%s\", \"dsdesc\": \"%s\", \"obstype\": \"%s\", \"uomname\": \"%s\", \"uomsym\": \"%s\", \"uomdef\": \"%s\", \"thingid\": \"%d\", \"obspropid\": \"%d\", \"sensorid\": \"%d\"}", 
                    mdsName.c_str(), mdsDescs[ii].c_str(), mdsObsTypes[ii].c_str(), mdsUomNames[ii].c_str(), mdsUomSyms[ii].c_str(), mdsUomDefs[ii].c_str(), thingId, mdsObsPropIds[ii], mdsSnrIds[ii]);
    
                Particle.publish("postMultiDatastream", postDs, PRIVATE);
                delay(5000);
            }
            
            delay(5000);
            Particle.subscribe(System.deviceID() + "/hook-response/getFilter/", checkThingStatus, MY_DEVICES);
            String filterStatus = String::format("Things(%d)?$select=id&$expand=Datastreams($select=id),MultiDatastreams($select=id)", thingId);
            String getStatus = String::format( "{ \"query\" : \"%s\"}", filterStatus.c_str());
            Particle.publish("getFilter", getStatus, PRIVATE);
            
            while (checked == -1) {
                delay(1000);
            }
            
            Particle.unsubscribe();
            Particle.subscribe(System.deviceID() + "/hook-response/getFilter/", retrieveThings, MY_DEVICES);
            String filterShow = String::format("Things(%d)?$select=id,name,properties&$expand=Datastreams($select=id,name),MultiDatastreams($select=id,name)", thingId);
            String getShow = String::format( "{ \"query\" : \"%s\"}", filterShow.c_str());
            Particle.publish("getFilter", getShow, PRIVATE);
            delay(1000);
        }
        
        else if (thingPropCnt >= 1) {
            Particle.unsubscribe();
            String dupDsMsg = String::format("%d DUPLICATES HAVE BEEN FOUND IN THE DATABASE, MAKE thingDesc PARAMETER UNIQUE", thingPropCnt);
            Particle.publish("INSTRUCTION", dupDsMsg, PRIVATE);
            delay(1000);
            
            Particle.subscribe(System.deviceID() + "/hook-response/getFilter/", retrieveThings, MY_DEVICES);
            String filterId = String::format("Things?$filter=name eq '%s' and description eq '%s'&$select=id", thingName.c_str(), thingDesc.c_str());
            String getId = String::format( "{ \"query\" : \"%s\"}", filterId.c_str());
            Particle.publish("getFilter", getId, PRIVATE);
            delay(1000);
        }
        
        thingCnt=-2;
    }
    
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

void countDeployment(const char *event, const char *data) {
    JSONValue outerObj = JSONValue::parseCopy(data);
    JSONObjectIterator iter(outerObj);
    while(iter.next()) {
        if (iter.name() == "@iot.count") {
            thingPropCnt = iter.value().toInt();
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
    
    if (dsCnt == nDs && mdsCnt == nMds) {
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

void retrieveThings(const char *event, const char *data) {
    Particle.publish("DETAILS", data, PRIVATE);
    delay(1000);
}