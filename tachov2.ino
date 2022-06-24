// Import required libraries
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <TM1650.h>

TM1650 d;



// Float Vars to store pressure values - NEW
String rpm = ""; //RPM
String vel = "";
int r_0,v_0,v,i= 0;

float RPM_last,lastTime,rpm_mean,rpm_mean_last,last_delay=0;
int temp;
const int timerDelay = 1;
const int mean_delay = 200;
const int n =mean_delay/timerDelay;
float RPM[n];
char a[]="0000";


// Replace with your network credentials - CHANGED
const char *ssid = "Tacho";
const char *password = "12345678";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


String processor(const String& var) {
  //Serial.println(var);
  if (var == "rpm") {
    return rpm;
  }
  else if (var == "vel") {
    return vel;
  }
  return String();
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .ds-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32 SoftAP</h2>
  <p>
    <span class="ds-labels">RPM x100</span><br> 
    <span id="rpm">%rpm%</span>
    <sup class="units">RPM</sup>
  </p>
    <p>
    <span class="ds-labels">Vel</span><br> 
    <span id="vel">%vel%</span>
    <sup class="units">kmh</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("rpm").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/rpm", true);
  xhttp.send();
    var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("vel").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/vel", true);
  xhttp.send();
}, 50) ;
</script>
</html>)rawliteral";


void setup() {

  Wire.begin();
  pinMode(4, INPUT_PULLUP); 
  // Wi-Fi Soft AP start
  WiFi.softAP(ssid, password);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/rpm", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", rpm.c_str());
  });
  // Start server
  server.begin();
  d.init();
  d.setBrightness(TM1650_MAX_BRIGHT);
  d.displayOn();
}

void loop() {

if ((millis() - lastTime) >= timerDelay) {

    if (digitalRead(4)== LOW and r_0 == 0){

      temp=RPM[n-1];
      for(i=n-1;i>0;i--){
          RPM[i]=RPM[i-1];
      }
      RPM[0]=1;
      //RPM=(RPM+rpm_mean_last)/2+1;
      r_0=1;

      for(i=0;i<n;i++){
          rpm_mean=rpm_mean+RPM[i];
      }
      rpm_mean=rpm_mean/(n+1);
      
    }
    
    if (digitalRead(4)== HIGH and r_0 == 1){

      temp=RPM[n-1];
      for(i=n-1;i>0;i--){
          RPM[i]=RPM[i-1];
      }
      RPM[0]=0;
      
      //RPM=(RPM+rpm_mean_last)/2;
      r_0=0;
      for(i=0;i<n;i++){
          rpm_mean=rpm_mean+RPM[i];
      }
      rpm_mean=rpm_mean/(n+1);
      
    }
    else{
      temp=RPM[n-1];
      for(i=n-1;i>0;i--){
          RPM[i]=RPM[i-1];
      }
      RPM[0]=0;
      
      for(i=0;i<n;i++){
          rpm_mean=rpm_mean+RPM[i];
      }
      rpm_mean=rpm_mean/(n+1);
    }

    lastTime = millis();
  
  }
  if ((millis() - last_delay) >= mean_delay) {
    
    rpm_mean=(rpm_mean+rpm_mean_last)/2;
    rpm_mean_last=rpm_mean;
    
    String(int(60*100*rpm_mean/timerDelay)).toCharArray(a,4);
    
    d.clear();
    d.displayString(a);
    rpm = String(int(60*1000*rpm_mean/timerDelay));
    vel = String(v);//read_vel();
    
    //Serial.println(60*1000*rpm_mean/timerDelay);

    last_delay = millis();

  }

 
}
