/*Sensorthings API integration with Particle Device
!!! indicates user input required=*/

/*=============================================================================================================================================
!!! Specify the Datastream and MultiDatastream Ids !!!
=============================================================================================================================================*/
int dsId_1st = 16;
int dsId_2nd = 17;
int mdsId_1st = 236;
//sepcify multi/datastreams depending on what is connected to the  

//!!! Specify the app that is flashed on the particle device
String appName = "stapi06_postobservations";
/*=============================================================================================================================================
END
=============================================================================================================================================*/
String dsIdString = String::format(
    "{ \"dsId1\" : \"%d\", \"dsId2\": \"%d\" }", 
    dsId_1st, dsId_2nd);
    
String mdsIdString = String::format(
    "{ \"mdsId1\" : \"%d\" }", 
    mdsId_1st);

void setup() {
    //Set the Datastream and MultiDatastream Ids as variables
    Particle.variable("datastreamId",dsIdString);
    Particle.variable("multidatastreamId", mdsIdString);
    Particle.variable("appName", appName);
}

void loop() {
    /*=============================================================================================================================================
    !!! Change these codes to cater to your specific sensors !!!
    !!! Currently they are only generating dummy results !!!
    =============================================================================================================================================*/
    //get the current time
    String timeNow = Time.format(Time.now(), TIME_FORMAT_ISO8601_FULL);
    //get the reading from the sensor
    float temp_1st = random(10, 50);
    float temp_2nd = random(20, 60);
    //get the multidatastream data 
    float lon = random(10, 50)*0.01;
    float lat = random(10, 50)*0.01;
    
    //post and publish the readings from the first sensor
    String postObsTemp = String::format(
    "{ \"time\" : \"%s\", \"result\": \"%f\", \"dsId\": \"%d\"}", 
    timeNow.c_str(), temp_1st, dsId_1st);
    Particle.publish("postObservation", postObsTemp, PRIVATE);
    
    //post and publish the readings from the second sensor 
    String postObsLux = String::format(
    "{ \"time\" : \"%s\", \"result\": \"%f\", \"dsId\": \"%d\"}", 
    timeNow.c_str(), temp_2nd, dsId_2nd);
    Particle.publish("postObservation", postObsLux, PRIVATE);
    
    //post and publish the readings from the multidatastream sensor
    String postMobsGPS = String::format(
    "{ \"time\" : \"%s\", \"result1\": \"%f\", \"result2\": \"%f\", \"mdsId\": \"%d\"}", 
    timeNow.c_str(), lon, lat, mdsId_1st);
    Particle.publish("postMultiObservation", postMobsGPS, PRIVATE);
    
    //decide the frequency of acquisition
    //default to 10seconds per acquisition
    delay(10000);
    /*=============================================================================================================================================
    END
    =============================================================================================================================================*/
}