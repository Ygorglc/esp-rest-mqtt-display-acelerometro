#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
//#include <ArduinoJson.h>
#include <SPI.h>
#include <SSD1306.h>
#include <ArduinoJson.h>
//#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Pinos do display (comunicação i2c)
const int DISPLAY_ADDRESS_PIN = 0x3c;
const int DISPLAY_SDA_PIN = 4;
const int DISPLAY_SCL_PIN = 15;
const int DISPLAY_RST_PIN = 16;
const int fontHeight = 16; 
// MQTT Broker
const char *mqtt_broker = "93.188.161.151";
const char *topic = "myTopic";
const char *mqtt_username = "ygor_admin";
const char *mqtt_password = "TESTE_1234";
const int mqtt_port = 1883;

#define OLED_RESET -1
//Adafruit_SSD1306 display(128, 32, &Wire);
SSD1306 display(DISPLAY_ADDRESS_PIN, DISPLAY_SDA_PIN, DISPLAY_SCL_PIN);
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
const char* serverName = "http://93.188.161.151:8085/defeito-motor-eletrico/v1/dados-gy87/dados-brutos";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

float ax  = 0; 
float ay  = 0;
float az  = 0;
float gx  = 0;
float gy  = 0;
float gz  = 0;
float t   = 0;
int   p   = 1;
int   idt = 1;
float mx  = 0;
float my  = 0;
float mz  = 0;
float b   = 0;


void setup() {

  Wire.begin(2, 0);
//  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  // initialize with the I2C addr 0x3D (for the 128x64)
//  display.clearDisplay();   // clears the screen and buffer 
  Wire.begin(2, 0);
  
//  display.setTextSize(1);
//  display.setTextColor(WHITE);
//  display.setCursor(0,0);
  if(!displayBegin())
  {
    // Se não deu certo, exibimos falha de display na serial
    Serial.println("Display failed!");
    // E deixamos em loop infinito
    while(1);
  }
  Serial.begin(115200);
  setup_wifi();

  setup_MQTT();
  
  //setup_MPU6050();

  time_to_action = millis();

  print_display();
  
}

bool displayBegin()
{
  // Reiniciamos o display
  pinMode(DISPLAY_RST_PIN, OUTPUT);
  digitalWrite(DISPLAY_RST_PIN, LOW);
  delay(1);
  digitalWrite(DISPLAY_RST_PIN, HIGH);
  delay(1);

  return display.init(); 
}

// Função que faz algumas configuções no display
void displayConfig()
{
  // Invertemos o display verticalmente
  display.flipScreenVertically();
  // Setamos a fonte
  display.setFont(ArialMT_Plain_16);
  // Alinhamos a fonta à esquerda
  display.setTextAlignment(TEXT_ALIGN_LEFT);
}


void Accer_Gyro(){
      int line = 0;
      unsigned long timexyz = millis();
      int i = 0;
      sensors_event_t a, g, temp;

      time_to_action = millis();
  
      display.clear();
      display.drawString(0, line, "Coletando Dados...");
      display.display();
      
      while(i!=10){ 
    
//        mpu.getEvent(&a, &g, &temp);

        accer_x[i] = 0;
        accer_y[i] = 0;
        accer_z[i] = 0;
        gyro_x_array[i] = 0;
        gyro_y_array[i] = 0;
        gyro_z_array[i] = 0;
//        accer_x[i] = a.acceleration.x;
//        accer_y[i] = a.acceleration.y;
//        accer_z[i] = a.acceleration.z;
//        gyro_x_array[i] = float(g.gyro.x);
//        gyro_y_array[i] = float(g.gyro.y);
//        gyro_z_array[i] = float(g.gyro.z);
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
  
      display.clear();
      display.drawString(0, line, "Dados Coletados!");
      display.display();
      
//      server.send(200, "application/json");
      Serial.print(F("done."));
}

void setup_MQTT(){
  int line = 0;
  time_to_action = millis();

      display.clear();
      display.drawString(0, line, "Setup mqtt!!!");
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
      display.clear();
      display.drawString(0, line, "Mqtt configurado!!!");
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
  int line = 0;
  time_to_action = millis();
  
      display.clear();
      display.drawString(0, line, "Setup mpu6050!!!");
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
      display.clear();
      display.drawString(0, line, "Configurado mpu6050!!!");
      display.display();
}

void setup_wifi(){
  int line = 0;
   time_to_action = millis();

  
      display.clear();
      display.drawString(0, line, "Setup wi-fi!!!");
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
  
      display.clear();
      display.drawString(0, line, "Configurado o wi-fi!!!");
      display.display();
  
}

void print_display(){
  int line = 0;
  time_to_action = millis();
  display.clear();
  display.drawString(0, line, "IP: ");
  line++;
  String ip = WiFi.localIP().toString();
  display.drawString(0, line * fontHeight, ip);
//  line++;
//  display.drawString(0, line, "Accer Range: ");
//  line++;
//  display.drawString(0, line, String(mpu.getAccelerometerRange()));
//  line++;
//  display.drawString(0, line, "Gyro Range: ");
//  line++;
//  display.drawString(0, line, String(mpu.getGyroRange()));
//  line++;
//  display.drawString(0, line, "Filter: ");
//  line++;
//  display.drawString(0, line, String(mpu.getFilterBandwidth()));
  display.display();

}

void rest_dados_bruto(){
  int line = 0;
  
  time_to_action = millis();

      display.clear();
      display.drawString(0, line, "Rest chamado!!!");
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

      while(i!=10){ 
        time_to_action = millis();
     
        display.clear();
        display.drawString(0, line, "Rest chamado posicao: ");
        display.drawString(0, (line+1) * fontHeight , String(i+1));
        display.display();
        
         //If you need an HTTP request with a content type: application/json, use the following:
          http.addHeader("Content-Type", "application/json");
          String httpRequestDataObjectJson = "{\"ax\": \""+String(ax)+"\",";
             httpRequestDataObjectJson += "\"ay\": \""+String(ay)+"\",";
             httpRequestDataObjectJson += "\"az\": \""+String(az)+"\",";
             httpRequestDataObjectJson += "\"gx\": \""+String(gx)+"\",";
             httpRequestDataObjectJson += "\"gy\": \""+String(gy)+"\",";
             httpRequestDataObjectJson += "\"gz\": \""+String(gz)+"\",";
             httpRequestDataObjectJson += "\"t\": \""+String(t)+"\",";
             httpRequestDataObjectJson += "\"p\": \""+String(p)+"\",";
             httpRequestDataObjectJson += "\"idt\": \""+String(idt)+"\",";
             httpRequestDataObjectJson += "\"mx\": \""+String(mx)+"\",";
             httpRequestDataObjectJson += "\"my\": \""+String(my)+"\",";
             httpRequestDataObjectJson += "\"mz\": \""+String(mz)+"\",";
             httpRequestDataObjectJson += "\"b\": \""+String(b)+"\"}";
                                            
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
   
      display.clear();
      display.drawString(0, line, "Rest finalizado!!!");
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
