#define DEBUG_STREAM SerialUSB
#define MODEM_STREAM Serial1
#define powerPin SARA_ENABLE
#define enablePin SARA_TX_ENABLE
#define STARTUP_DELAY 5000
#define controllerPin A7
#include <Sodaq_UBlox_GPS.h>
#include "Sodaq_wdt.h"

// VARIABLE INITIALIZATIONS

String gps = "";
int red_light_pin= 2;
int green_light_pin = 3;
int blue_light_pin = 4;

// APN and forceoperator below lpwa.telia.iot is the APN for Telia, TDC, and other operators have different ones
const char* apn = "lpwa.telia.iot";
const char* forceOperator = "23820"; 

const char* cdp = "";
// BELOW IS THE TOKEN FOR THINGS.IO
const String token = "014439733952576939532d7a776f533247714e5171356d5a46765954763642456951773833696b35484d5234";


uint8_t cid = 0;
const uint8_t band = 20;
unsigned long baud = 9600;

// BELOW IS THE IP AND PORT FOR THE ENDPOINT
const String ip = "\"104.199.85.211\"";
String port = "28399";
int con;

String msg;
String res;
String readstring;
char lat_buff[16];
char lon_buff[16];


// HELPER FUNCTION TO SET RGB OF LED. LIGHT VALUE BETWEEN 0-255
void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
  analogWrite(red_light_pin, red_light_value);
  analogWrite(green_light_pin, green_light_value);
  analogWrite(blue_light_pin, blue_light_value);
}


// THE AT COMMAND AT+NSOST, WHICH SENDS UDP DATAGRAM TO THE SPECIFIED ENDPOINT
void send_message(int prt,String msg){
  String message = token + msg;
  // THE SIZE IS HALVED BECAUSE WE'RE DEALING WITH BYTES 
  int size_msg = message.length()/2;
  MODEM_STREAM.println(String("AT+NSOST=")+prt+','+ip+','+port+','+ size_msg + ','+'"'+message+'"');
  delay(1000);
  readstring ="";
  while (MODEM_STREAM.available() > 0){
    char c = MODEM_STREAM.read(); 
    DEBUG_STREAM.println(MODEM_STREAM.readString());
    readstring +=c;
  }
  int ind = readstring.indexOf("OK");
  if (ind == -1){
    DEBUG_STREAM.println(readstring);
  }
  else {
    DEBUG_STREAM.println("SUCCESS:");
    DEBUG_STREAM.println(readstring);
  }
}

// STUFF TO BE DONE BEFORE SENDING DATAGRAM
void prepare_send(String message){
  readstring = "";
  int port;
  MODEM_STREAM.flush();
  
  // OPENS A SOCKET ON THE BOARD
  MODEM_STREAM.println("AT+NSOCR=\"DGRAM\",17,42000");
  delay(100);

  // LOOP OVER INPUT STREAM; IGNORE NEW LINES
  while (MODEM_STREAM.available() > 0){
    char c = MODEM_STREAM.read();
    if (c == '\n'){
      // ignore
    }
    else {
      // ADDS TO THE readstring VARIABLE
      readstring +=c;
    }
  }

  // THE INDEX IS RETURNED FROM THE BOARD WE EXPECT 0 AS IT IS THE ONLY PORT IN USE.
  int index = readstring.indexOf("0");
  if (index == -1){
    DEBUG_STREAM.println("Can't open");
    port = readstring.charAt(0);
  }
  else {
    port = 0;
  }
  send_message(port, message);
  close_port(port);
}

// CLOSE THE PORT AFTER USE
void close_port(int p){
  readstring ="";
  MODEM_STREAM.flush();
  MODEM_STREAM.println(String("AT+NSOCL=")+p);
  delay(100);
  while (MODEM_STREAM.available() > 0){
    char c = MODEM_STREAM.read();
    if (c == '\n'){
      //
    }
    else {
      readstring += c;
    }
  }
  DEBUG_STREAM.println(readstring);
  if (readstring.indexOf("OK") == -1){
    DEBUG_STREAM.println("FAIL");
  } else{
    DEBUG_STREAM.println("Deleted socket");
  }
  
}
// WE BUILD OUR STRING DATAGRAM HERE
// THE MOST IMPORTANT FUNCTION
String build_message(){ 
  String infostring =""; 
  String ptid;
  String tx_val;
  String gps_string;
  int info_counter = 0;
  String info_val;
  String lat;
  String lon;
  // IF THERE IS A GPS LOCATION FOUND(FROM SETUP)
  if(gps.length() > 0){
    
    // CONVERTING IT TO HEXADECIMAL FORMAT
    lat = gps.substring(0,6);
    lon = gps.substring(7,13);
    lat.toCharArray(lat_buff,16);
    lon.toCharArray(lon_buff,16);
    float lat_float = atof(lat_buff);
    float lon_float = atof(lon_buff);

    // Multiply by 10,000 to get rid of decimal points
    lat_float = lat_float*10000;
    lon_float = lon_float*10000;
    int lat_int = (int) lat_float;
    int lon_int = (int) lon_float;
    
    gps_string = "0"+String(lat_int,HEX)+"0"+String(lon_int,HEX);
    }
    
    else{
    gps_string = "";
    }
    DEBUG_STREAM.println("GPS");
    DEBUG_STREAM.println(gps_string);
     MODEM_STREAM.println("AT+NUESTATS");
    delay(100);
    while (MODEM_STREAM.available() > 0){
      char c = MODEM_STREAM.read();
      if (c == '\n' && info_counter > 1){
      }
      else{
        info_counter +=1;
        infostring += c;
      }
    }
    delay(100);
    while (MODEM_STREAM.available() > 0){
      char c = MODEM_STREAM.read();
      if (c == '\n' && info_counter > 1){
      }
      else{
        info_counter +=1;
        infostring += c;
      }
    }


  // ADD NUESTATS RESPONSE TO INFO STRING: AND LOOK FOR "CELL ID" "SIGNAL POWER" SUBSTRINGS
  int info_index = infostring.indexOf("\"Cell ID\",");
  int tx_index = infostring.indexOf("\"Signal power\",");
  tx_val = infostring.substring(tx_index+16,tx_index+20);
  info_val = infostring.substring(info_index+10,info_index+18);
    
  int tx_val_int = tx_val.toInt();
  int info_val_int = info_val.toInt();
  
  String s1 = String(tx_val_int,HEX);
  String s2 = String(info_val_int,HEX);
  String result;
  readstring ="";
  MODEM_STREAM.println("AT+CSQ");
  delay(100);
  int counter = 0;
  while (MODEM_STREAM.available() > 0){
    char c = MODEM_STREAM.read();
    if (c == '\n' && counter > 1){
    }

    else{
      counter +=1;
      readstring += c;
    }
  }
  delay(100);
  while (MODEM_STREAM.available() > 0) {
    MODEM_STREAM.read();
  }
  
  int index1 = readstring.indexOf(",");
  String val = readstring.substring(index1-2,index1);
  int signal_dbm = val.toInt();
  
  // control led light depending on the result from "AT+CSQ"
  if(0 < signal_dbm && signal_dbm <=10){
    RGB_color(200,0,0);
  }
  else if (10 < signal_dbm && signal_dbm <= 21){
    RGB_color(0,0,200);
  }
  else if(21 < signal_dbm && signal_dbm <=31){
    RGB_color(0,200,0);
  }
  else{
    RGB_color(0,0,0);
  }
  
  val = String(signal_dbm,HEX);

  // WE NEED TO MANUALLY ADD "0" TO HEX BYTES IF NOT PRESENT | we need to have an even sized message.
  String prepend = "0";
  if(val.length() % 2 == 0){
      result = val;
  }
  else{
      result = prepend+val;
  }
  return(result+gps_string+prepend+s2+prepend+s1);
}

// CHECKING IF THE NB-MODULE IS CONNECTED TO THE NETWORK
// SHOULD RETURN "1" IF IT IS "0" OTHERWISE
bool is_attached(){
  readstring = "";
  MODEM_STREAM.println("AT+CGATT?");
  delay(100);
  while (MODEM_STREAM.available() > 0){
    char c = MODEM_STREAM.read(); 
    if (c == '\n'){
    }
    else {
      readstring += c;
    }
  }
  if (readstring.length() > 0){
    int index = readstring.indexOf("1");
    if (index == -1){
      return false;
    }
    // IF "1" IS RETURNED WE RETURN TRUE
    else{
      return true;
    }
  }
}

// THE INITIAL CONNECTIVITY FUNCTION
void nbconnect(){
  while (true){
    readstring = "";
    MODEM_STREAM.println("AT+CFUN=1");
    // check for "OK"
    delay(STARTUP_DELAY);
    while (MODEM_STREAM.available() > 0){
      char c = MODEM_STREAM.read();
      readstring += c;
    }
    int x = readstring.indexOf("OK");
    if (x == -1){
      //keep trying cfun
    }
    else {
      MODEM_STREAM.flush();
      break;
    }
  }
  readstring = "";
  MODEM_STREAM.println("AT+COPS=0");
  delay(20);
  while (MODEM_STREAM.available() > 0){
    char c = MODEM_STREAM.read(); 
    if (c == '\n' ){
    }
    else  {
      readstring +=c;
    }
    
  }
  if (readstring.indexOf("OK") == -1){
    DEBUG_STREAM.println("FAILED COPS = 0");
  }
  else {
    con = 1;
  }
}


void setup() {
  pinMode(controllerPin, INPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(red_light_pin, OUTPUT);
  pinMode(green_light_pin, OUTPUT);
  pinMode(blue_light_pin, OUTPUT);
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, HIGH);

  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);

  sodaq_wdt_safe_delay(STARTUP_DELAY);
  // put your setup code here, to run once:
  DEBUG_STREAM.begin(baud);
  MODEM_STREAM.begin(baud);
  sodaq_gps.init(GPS_ENABLE);

  nbconnect();
  //sodaq_gps.setMinNumOfLines(10);
  //sodaq_gps.setDiag(DEBUG_STREAM);

  gps = find_fix(0);
}

// GPX FUNCTION CAN REMOVE AND USE APP IF UNNECESSARY
String find_fix(uint32_t delay_until)
{
    String result = "";
    RGB_color(255,255,255);
    uint32_t start = millis();
    delay(delay_until);
    uint32_t timer = 120*1000;
    uint32_t timeout = 30 * 1000;
    while( (millis() - start) < timer){
        if (sodaq_gps.scan(false, timeout)) {
          RGB_color(0,255,255);

          // UNCOMMENT FOR DEBUG STUFF
          
        //DEBUG_STREAM.println(String(" time to find fix: ") + (millis() - start) + String("ms"));
        //DEBUG_STREAM.println(String(" datetime = ") + sodaq_gps.getDateTimeString());
       // DEBUG_STREAM.println(String(" lat = ") + String(sodaq_gps.getLat(), 4));
        //DEBUG_STREAM.println(String(" lon = ") + String(sodaq_gps.getLon(), 4));
        //DEBUG_STREAM.println(String(" num sats = ") + String(sodaq_gps.getNumberOfSatellites()));

        // 4 digits in Lat and 4 in Long, 
        // TO INCREASE OR DECREASE PRECISION CLOUD CODE SHOULD ALSO BE CHANGED IN ACCORDANCE 
        String lat = String(sodaq_gps.getLat(),4);
        String lon = String(sodaq_gps.getLon(),4);
        result = lat+lon;
        return result;
    } 
    else {
        DEBUG_STREAM.println("No Fix");
    }
    }
    RGB_color(0,0,0);
    //return result;
    return "";
}

void loop() {
  if (!(con == 1)){
    DEBUG_STREAM.println(con);
    nbconnect();
  }
  delay(30000);
  if(is_attached()){
    DEBUG_STREAM.println("Connected");
    String csq = build_message();
    prepare_send(csq);
  }
  else {
    DEBUG_STREAM.println("Not connected");
  }
  
}