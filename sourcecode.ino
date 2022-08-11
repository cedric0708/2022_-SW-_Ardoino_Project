#include <SoftwareSerial.h>
#include <stdlib.h>
#define DEBUG true

byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
byte response[9];
String ppmString = " ";

int ledPin = 10;
int ledPin2 = 9;
int lm35Pin = 13; //터치센서
int cnt=0;  //터치센서 작동 횟수
int ppm;

 
// 자신의 thingspeak 채널의 Write API key 입력
String apiKey = "N59XZYWDU2E2WADU";
 
SoftwareSerial esp8266(2,3); // TX/RX 설정, esp8266 객체생성
SoftwareSerial mySerial(A0, A1);

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  //시리얼통신속도 9600보드레이트 설정    
  Serial.begin(9600); 
  //소프트웨어 시리얼 시작
  esp8266.begin(9600);
  mySerial.begin(9600);
  /*AT Command 이용*/
  sendData("AT+RST\r\n", 2000, DEBUG); //reset module
  sendData("AT+CWMODE=1\r\n", 1000, DEBUG); //dual mode로 설정
  sendData("AT+CWJAP=\"SK_WiFiGIGA24BA_5G\",\"1601020573\"\r\n", 5000, DEBUG); //사용할 공유기 설정
}

 
void loop() {
  digitalWrite(ledPin, HIGH);
  for(int i=0;i<16;i++){
    int val = digitalRead(lm35Pin);
    if(val==HIGH)
    {
      cnt++;             
    }
    if(cnt%2==1)
    {
      digitalWrite(ledPin2, HIGH);
    }
    else{
      digitalWrite(ledPin2, LOW);
    }
    delay(1000);
  }
   digitalWrite(ledPin, LOW);
  
   if(cnt%2==1)
    {
      Serial.print("Power ON\n");
    }
    else{
      Serial.print("Power OFF\n");
    }
  
  if(cnt%2==1){
  mySerial.listen();
  mySerial.write(cmd,9);
  mySerial.readBytes(response, 9);
  byte chck = 0;
  
  if(response[8] == (0xff&(~(response[1]+response[2]+response[3]+response[4]+response[5]+response[6]+response[7]) + 1))){
    Serial.println("OK");
  }
  else {
    Serial.print("chksum : ");
    Serial.println(response[8],HEX);
    Serial.print("read : ");
    Serial.println(0xff&(~(response[1]+response[2]+response[3]+response[4]+response[5]+response[6]+response[7]) + 1),HEX);
    while(mySerial.available() > 0){
      mySerial.read();
    }
  }
  
  ppm = (response[2] << 8)|response[3];
  ppmString = String(ppm); //int to string
  
  Serial.print("PPM ");
  Serial.println(ppm);
  }
  
  else
  {
    ppm=0;
  }

  
  // TCP 연결
  esp8266.listen();
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com 접속 IP
  cmd += "\",80";           // api.thingspeak.com 접속 포트, 80
  esp8266.println(cmd);
   
  if(esp8266.find("Error")){
    Serial.println("AT+CIPSTART error");
    return;
  }
  
  // GET 방식으로 보내기 위한 String, Data 설정
  String getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr +="&field1=";
  getStr += ppm;
  getStr += "\r\n\r\n";
 
  // Send Data
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  esp8266.println(cmd);
 
  if(esp8266.find(">")){
    esp8266.print(getStr);
  }
  else{
    esp8266.println("AT+CIPCLOSE");
    // alert uesp8266
    Serial.println("AT+CIPCLOSE");
  }
    
  // Thingspeak 최소 업로드 간격 15초를 맞추기 위한 delay
  
 }


/*ESP8266의 정보를 알아내고 설정하기 위한 함수 선언*/
String sendData(String command, const int timeout, boolean debug){
  String response = "";
  esp8266.print(command); //command를 ESP8266에 보냄
  long int time=millis();
  
  while((time+timeout)>millis()){
    while(esp8266.available()){
      /*esp가 가진 데이터를 시리얼 모니터에 출력하기 위함*/
      char c=esp8266.read(); //다음 문자를 읽어옴
      response+=c;
    }
  }
  if(debug){
    Serial.print(response);
  }
 
  return response;
}
