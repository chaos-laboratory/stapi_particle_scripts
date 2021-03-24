/*Sensorthings API integration with Particle Device
!!! indicates user input required=*/

int thingCnt = -1;
String deviceId = System.deviceID();


void setup() {
    //Subscribe to the event stream so we can response in the countThing function
    Particle.subscribe(System.deviceID() + "/hook-response/getFilter/", countThing, MY_DEVICES);
    //Particle.subscribe("hook-response/getFilter", countThing, MY_DEVICES);
}

void loop() {
    if (thingCnt == -1) {
        /*Check if this particle device is already registered in the database
        a sensorThing query is sent thru the 'getFilter' webhook, 
        we will receive a JSON reply from our database
        the appropriate reaction is coded in the countThing function*/
        String filterCount = String::format("Things?$filter=properties/uid eq '%s'&$select=id&$count=true&$skip=1000000", deviceId.c_str());
        String getCount = String::format( "{ \"query\" : \"%s\"}", filterCount.c_str());
        Particle.publish("getFilter", getCount, PRIVATE);
        delay(10000);
    }
    
    else if (thingCnt == 0 ) {
        Particle.publish("NOT_REGISTERED", "THIS DEVICE HAS NOT BEEN REGISTERED, FLASH THE STAPI02_RegisterDevice TO REGISTER YOUR DEVICE", PRIVATE);
        thingCnt = -2;
        //Particle.unsubscribe();
    }
    
    else if (thingCnt == 1 ) {
        Particle.unsubscribe();
        //The device is already registered post all the information about the device
        //Subscribe to the event stream so we can response in the checkThingStatus function
        Particle.subscribe(System.deviceID() + "/hook-response/getFilter/", checkThingStatus, MY_DEVICES);
        /*Check if this particle device is already registered in the database
        a sensorThing query is sent thru the 'getFilter' webhook, 
        we will receive a JSON reply from our database
        the appropriate reaction is coded in the checkThingStatus function*/
        String filterThing = String::format("Things?$filter=properties/uid eq '%s'&$select=id,properties&$expand=Datastreams($select=id,name),MultiDatastreams($select=id,name)", deviceId.c_str());
        String getThing = String::format( "{ \"query\" : \"%s\"}", filterThing.c_str());
        Particle.publish("getFilter", getThing, PRIVATE);
        delay(10000);
        thingCnt = -2;
    }
    
    else if (thingCnt > 1) {
        Particle.unsubscribe();
        //The device has been registered more than once in the database
        String dupDsMsg = String::format("%d DUPLICATES HAVE BEEN FOUND IN THE DATABASE", thingCnt);
        Particle.publish("INSTRUCTION", dupDsMsg, PRIVATE);
        delay(1000);
        Particle.publish("INSTRUCTION", "FOR NEW DEPLOYMENT, USE STAPI03_RegisterNewDeployment", PRIVATE);
        delay(1000);
        Particle.subscribe(System.deviceID() + "/hook-response/getFilter/", checkThingStatus, MY_DEVICES);
        /*Check if this particle device is already registered in the database
        a sensorThing query is sent thru the 'getFilter' webhook, 
        we will receive a JSON reply from our database
        the appropriate reaction is coded in the checkThingStatus function*/
        String filterThing = String::format("Things?$filter=properties/uid eq '%s'&$select=id", deviceId.c_str());
        String getThing = String::format( "{ \"query\" : \"%s\"}", filterThing.c_str());
        Particle.publish("getFilter", getThing, PRIVATE);
        delay(1000);
        thingCnt = -2;
    }
}

void checkThingStatus(const char *event, const char *data) {
    Particle.publish("REGISTRATION_DETAILS", data, PRIVATE);
    delay(1000);
    Particle.publish("REGISTERED", "USE THE DETAILS TO POST OBSERVATIONS IN THE STAPI06_PostObservation SCRIPT", PRIVATE);
    delay(1000);
}

void countThing(const char *event, const char *data) {
    //Particle.publish("data", data, PRIVATE);
    JSONValue outerObj = JSONValue::parseCopy(data);
    JSONObjectIterator iter(outerObj);
    while(iter.next()) {
        if (iter.name() == "@iot.count") {
            thingCnt = iter.value().toInt();
        }
    }
}