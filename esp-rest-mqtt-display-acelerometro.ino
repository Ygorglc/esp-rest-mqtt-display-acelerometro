#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
//#include <ArduinoJson.h>
#include <SPI.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// MQTT Broker
const char *mqtt_broker = "93.188.161.151";
const char *topic = "myTopic";
const char *mqtt_username = "ygor_admin";
const char *mqtt_password = "TESTE_1234";
const int mqtt_port = 1883;

#define OLED_RESET -1
Adafruit_SSD1306 display(128, 32, &Wire);
#define OLED_address  0x3c 

const char* ssid = "YGORR";
const char* password = "12345ygor";
//
//IPAddress ip(192,168,1,25);
//IPAddress gateway(192,168,1,1); 
//IPAddress subnet(255,255,255,0);

//ESP8266WebServer server(80);

Adafruit_MPU6050 mpu;
float accer_x[1000],
       accer_y[1000],
       accer_z[1000],
       temperature;
float  gyro_x_array[1000],
       gyro_y_array[1000],
       gyro_z_array[1000],
       time_xyz_array[1000];
float time_to_action;


WiFiClient espClient;

PubSubClient client(espClient);

//Your Domain name with URL path or IP address with path
const char* serverName = "http://93.188.161.151:8085/defeito-motor-eletrico/v1/dados/dados-brutos";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

float aceleracao_eixo_x = 0; 
float aceleracao_eixo_y = 0;
float aceleracao_eixo_z = 0;
float giro_eixo_x = 0;
float giro_eixo_y = 0;
float giro_eixo_z = 0;
float tempo = 0;
int posicao = 0;
int idTeste = 2;

void setup() {

  Wire.begin(2, 0);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.clearDisplay();   // clears the screen and buffer 
  Wire.begin(2, 0);
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  
  Serial.begin(115200);
  setup_wifi();

  setup_MQTT();
  
  setup_MPU6050();

  time_to_action = millis();

  print_display();
  
}

void Accer_Gyro(){
      unsigned long timexyz = millis();
      int i = 0;
      sensors_event_t a, g, temp;

      time_to_action = millis();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
  
      display.clearDisplay();
      display.print("Coletando Dados...");
      display.println();
      display.display();
      
      while(i!=800){ 
    
        mpu.getEvent(&a, &g, &temp);

        accer_x[i] = a.acceleration.x;
        accer_y[i] = a.acceleration.y;
        accer_z[i] = a.acceleration.z;
        gyro_x_array[i] = float(g.gyro.x);
        gyro_y_array[i] = float(g.gyro.y);
        gyro_z_array[i] = float(g.gyro.z);
        time_xyz_array[i] = millis()-timexyz;
        
        i++;
        delay(10);
      }
      temperature = temp.temperature;
      //DynamicJsonDocument doc(512);
      
      Serial.print(F("Stream..."));
      String buf;
      //serializeJson(doc, buf);

      time_to_action = millis();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
  
      display.clearDisplay();
      display.print("Dados Coletados!");
      display.println();
      display.display();
      
//      server.send(200, "application/json");
      Serial.print(F("done."));
}

void setup_MQTT(){
  time_to_action = millis();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
  
      display.clearDisplay();
      display.print("Setup mqtt!!!");
      display.println();
      display.display();
      
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  
  while (!espClient.connected()) {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Public emqx mqtt broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
  }
  // publish and subscribe
  client.publish(topic, "myTopic");
  client.subscribe(topic);
  
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");

  time_to_action = millis();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
  
      display.clearDisplay();
      display.print("Mqtt configurado!!!");
      display.println();
      display.display();
}

void callback(char *topic, byte *payload, unsigned int length) {
  String idTeste;
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
      Serial.print((char) payload[i]);
      idTeste+= (char) payload[i];
  }
  Serial.println();
  Serial.println("-----------------------");
  Serial.println(idTeste);
  Serial.println("-----------------------");
  
  Accer_Gyro();
  rest_dados_bruto();
}

void setup_MPU6050(){
  time_to_action = millis();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
  
      display.clearDisplay();
      display.print("Setup mpu6050!!!");
      display.println();
      display.display();
      
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  delay(100);
  
  time_to_action = millis();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
  
      display.clearDisplay();
      display.print("Configurado mpu6050!!!");
      display.println();
      display.display();
}

void setup_wifi(){
   time_to_action = millis();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
  
      display.clearDisplay();
      display.print("Setup wi-fi!!!");
      display.println();
      display.display();
      
//  WiFi.config(ip, gateway, subnet);
//  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Activate mDNS this is used to be able to connect to the server
  // with local DNS hostmane esp8266.local
//  if (MDNS.begin("esp8266")) {
//    Serial.println("MDNS responder started");
//  }
  time_to_action = millis();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
  
      display.clearDisplay();
      display.print("Configurado o wi-fi!!!");
      display.println();
      display.display();
  
}

void print_display(){

  time_to_action = millis();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  
  display.clearDisplay();
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.print("Accer Range: ");
  display.println(mpu.getAccelerometerRange());
  display.print("Gyro Range: ");
  display.println(mpu.getGyroRange());
  display.print("Filter: ");
  display.println(mpu.getFilterBandwidth());
  display.display();

}

void rest_dados_bruto(){
  time_to_action = millis();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
  
      display.clearDisplay();
      display.print("Rest chamado!!!");
      display.println();
      display.display();
   HTTPClient http;
   int i = 0;
      
      // Your Domain name with URL path or IP address with path
      http.begin(espClient, serverName);

      // Specify content-type header
//      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
//      // Data to send with HTTP POST
//      String httpRequestData = "api_key=tPmAT5Ab3j7F9&sensor=BME280&value1=24.25&value2=49.54&value3=1005.14";           
//      // Send HTTP POST request
//      int httpResponseCode = http.POST(httpRequestData);

      while(i!=800){ 
        time_to_action = millis();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
    
        display.clearDisplay();
        display.print("Rest chamado posicao: ");
        display.print(i);
        display.println();
        display.display();
         //If you need an HTTP request with a content type: application/json, use the following:
          http.addHeader("Content-Type", "application/json");
          String httpRequestDataObjectJson = "{\"aceleracao_eixo_x\": \""+String(accer_x[i])+"\",";
                 httpRequestDataObjectJson +="\"aceleracao_eixo_y\": \""+String(accer_y[i])+"\",";
                 httpRequestDataObjectJson += "\"aceleracao_eixo_z\": \""+String(accer_z[i])+"\",";
                 httpRequestDataObjectJson += "\"giro_eixo_x\": \""+String(gyro_x_array[i])+"\",";
                 httpRequestDataObjectJson += "\"giro_eixo_y\": \""+String(gyro_y_array[i])+"\",";
                 httpRequestDataObjectJson += "\"giro_eixo_z\": \""+String(gyro_z_array[i])+"\",";
                 httpRequestDataObjectJson += "\"tempo\": \""+String(time_xyz_array[i])+"\",";
                 httpRequestDataObjectJson += "\"posicao\": \""+String((i+1))+"\",";
                 httpRequestDataObjectJson += "\"idTeste\": \""+String(idTeste)+"\"}";
                                            
          //String httpRequestDataObjectJson = "{\"descricao\":\"teste-esp8266-rest-post\"}";
          int httpResponseCode = http.POST(httpRequestDataObjectJson);
    
          // If you need an HTTP request with a content type: text/plain
          //http.addHeader("Content-Type", "text/plain");
          //int httpResponseCode = http.POST("Hello, World!");
         
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
          i++;
          //delay(5);
      }
      
      // Free resources
      http.end(); 
      time_to_action = millis();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
  
      display.clearDisplay();
      display.print("Rest finalizado!!!");
      display.println();
      display.display();
}

void loop() {
  if(millis()- time_to_action>10000){
    print_display();
    time_to_action = millis();
  }
  //Send an HTTP POST request every 10 minutes
  //if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
       client.loop();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  //}
  if(!client.connected()){
    setup_MQTT();
  }
}
