// SimOver.Net M590 SerialController

#include <SoftwareSerial.h>
//#include <EEPROM.h>

//#define GPRS_APN "internet.tele2.ru"
//#define GPRS_USER ""
//#define GPRS_PASS ""
//#define GPRS_NUMBER "*99***1#"
// GSM UART PINs
#define GSM_RX 7
#define GSM_TX 8
#define GSM_PORT_SPEED 9600
#define POWERPIN 13
#define SERIAL_PORT_SPEED 9600

#define terminator 10 // DEC value for a LF(line feed) to skip while loop

void gprsconnect();
String host(String);

/*
  SETUP
  See https://arbitraryuser.com/2013/04/07/seeed-studio-gprs-shield-and-arduino-mega-2560/
  This code is partially "borrowed" from idS2001 at http://www.seeedstudio.com/forum/viewtopic.php?p=12939#p12939
*/

String IncDataSerial = "";
String simoverIP="8.8.8.8";


SoftwareSerial gsm(GSM_RX, GSM_TX); // RX, TX


void gprsconnect() {
  //gsm.println("AT+CGDCONT=1,\"IP\",\"internet.tele2.ru\"");
  gsm.println('AT+CGDCONT=1,"IP","internet.tele2.ru"');
  delay(100);
  //gsm.println("AT+XGAUTH=1,1,\""+GPRS_USER+"\",\""+GPRS_PASS+"\"");
  //gsm.println("AT+XGAUTH=1,1,\"\",\"\"");
  gsm.println('AT+XGAUTH=1,1,"",""');
  delay(100);
  gsm.println("AT+CUSD=1,\"*99***1#\",15");
  delay(100);
  gsm.println("AT+XISP=0");  // DialUP
  delay(100);
  gsm.println("at+xiic=1");
  delay(100);
  do {            // wait IP
    gsm.println("at+xiic?");
    serial_char(".");
    delay(300);
  } while (gsm.find("0.0.0.0"));
  delay(1000);
}

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(SERIAL_PORT_SPEED);
  //  while (!Serial) {
  //    ; // wait for serial port to connect. Needed for native USB port only
  //  }
  serial_line("/**");
  serial_line(" * www.simover.net");
  serial_line(" */");

  // set the data rate for the SoftwareSerial port
  /**
   * MODEM:STARTUP
   * ...
   * +PBREADY
   */
  while (!gsm) {
    if (digitalRead(POWERPIN) == LOW) {
      digitalWrite(POWERPIN, HIGH);
    } else {
      digitalWrite(POWERPIN, LOW);
    }
    delay(700);
  }

  gsm.begin(GSM_PORT_SPEED);
  
  // Initialize modem

  delay(1000);
  serial_line("// Initialize modem");
  gsm.println("ATZ");
  delay(300);
  gsm.println("AT+CPIN?");
  do {
    //serial_line("Disable PIN Code");
    delay(300);
  } while(!gsm.find("READY"));

  delay(300);
  gsm.println("AT+CLIP=1");
  delay(300);
  /**
     AT+CMGF — формат сообщений, 0-1.
     0, — режим PDU, управление кодом команды. Вывод сообщения в HEX коде. Режим по умолчанию
     1, — текстовый режим. Команды текстовые. Вывод сообщения в текстовом виде.
     В первый режиме сообщения будут выдаваться в виде шестнадцатеричных кодов ascii или unicode. Очень, очень не удобоваримый режим.
  */
  gsm.println("AT+CMGF=1");
  delay(300);
  /**
     AT+CSCB=1, прием широковещательных сообщений. Это та гадость, через которую сейчас срут все операторы. По умолчанию включена,
     для исправления этой недоработки используем указанную команду.
  */
  gsm.println("AT+CSCB=1");
  delay(300);
  /**     
   * USSD запросы.
   * Для проверки баланса и разных настроек обычно используются ussd запросы. Очень часто запросы начинающиеся со '*' возвращают ответ в юникоде, на родном языке. К счастью, многие телефоны не понимают этой кодировки и для них были введены запросы начинающиеся с '#'. Они возвращают ответ в стандартном ascii.
   * Сделать запрос можно 2 способами.
   * Первый, используя специальную команду:
   * AT+CUSD=1,"#102#"
   *  1, — режим обработки ответа
   *  0 — выполнить запрос, полученный ответ проигнорировать
   *  1 — выполнить запрос, ответ вернуть в терминал
   *  2 — отменить операцию
   * "#102#" — само сообщение. Отправляется только то, что находится в скобках.
   * Второй выглядит как обычный набор номера. Результат всегда возвращается в терминале. Но работает не на всех версиях прошивки.
   *  ATD#102#;
   */
  simoverIP=host("simover.net");
  
  serial_line(">>>");
  serial_line(simoverIP);
  serial_line("<<<");
  get("/");
}

String currStr = "";
int updateTime = 0;
String callerID = "";

void loop()
{  
  // If Serial established, debug mode
  if(Serial){
    if (Serial.available()) {
      gsm.write(Serial.read());
    }
    if (gsm.available()) {
      Serial.write(gsm.read());
    }
  }
  if(gsm) {
    
  }
}


void sendat()
{
  if (millis() >= updateTime) {
    gsm.println("AT");
    updateTime += 60000;
  }
}

void serial_char(char *c) {
  if(Serial) {
    Serial.write(c);
  }
}
void serial_line(String str) {
  if (Serial) {
    Serial.println(str);
  }
}

bool gprscheck() {
  gsm.flush(); 
  gsm.println("at+xiic?");
  delay(100);  
  if (gsm.find("0.0.0.0")) gprsconnect(); // если нет, то подключаемся
  gsm.flush();      
}

String IP;

String host(String domain) {
  gprscheck();
  gsm.println("at+dns=\""+domain+"\"");
  delay(500);
  do {
    delay(300);
  }while(!gsm.find("DNS:"));

  IP = gsm.readString();
  
  if(IP.indexOf("\n") > 1) {
    IP = IP.substring(0, IP.indexOf("\n"));
  }
  IP.trim();
  return IP;
}

String http_request(String request) {
  serial_line("Request:");
  serial_line(request);
  gsm.println("AT+TCPSETUP=1,"+simoverIP+",80");
  do {
    delay(100);
  }while(gsm.find("+TCPSETUP:"));
  if(gsm.readString().indexOf("OK")) {
    gsm.println("AT+TCPSEND=1,"+request.length());
    delay(300);
    gsm.println(request);
    delay(300);
  }
}

String get(String url) {
  http_request("GET "+url+" HTTP/1.0\r\nHost: simover.net\r\n");
}

String post(String url, String data) {
  http_request("POST "+url+" HTTP/1.0\r\nHost: simover.net\r\n\r\n"+data);
}

