  #include <SPI.h>   // El shield Etherent usa SPI
  #include <Ethernet.h>
  #include "DHT.h"
  #include <LiquidCrystal.h>
  #include <Servo.h>
  
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  IPAddress ip(192, 168, 137, 5); // vuestra IP
  EthernetServer server(80);
  Servo microservo;
  
  //LCD
  const int rs = 5, en = 6, d4 = 7, d5 = 8, d6 = 9, d7 = 10;
  LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
  #define COLS 16
  #define ROWS 2
  
  // Definir el tipo de DHT
  #define DHTTYPE DHT11
  
  //Buzzer y reed switch
  #define reed 1
  #define buzzer 12
  const int tono1 = 1000;
  const int tono2 = 1250;
  volatile int value = HIGH;
  const int buz = 12;
  int persona = 0;
  
  
  //Definir el pin al que vamos a conectar el DHT11
  const int DHTPin = 2;
  const int Trigger = 3;   //Pin digital 2 para el Trigger del sensor
  const int Echo = 4;   //Pin digital 3 para el Echo del sensor
  
  
  //Definir el pin y el tipo de DHT para obtener la humedad y temperatura
  DHT dht(DHTPin, DHTTYPE);
  
  //Variables para guardar la humedad y temperatura
  float humedad, temperatura = 0;
  int llama = 0;
  long d, t;
  String readString;
  
  void setup()
  {
  
    pinMode(reed, INPUT_PULLUP);
    pinMode(buzzer, OUTPUT);
    microservo.attach(11);
    microservo.write(180);
  
    Serial.begin(9600);
    lcd.begin(COLS, ROWS);
  
    lcd.print("La querendona");
    pinMode(12, OUTPUT);
  
  
    Serial.begin(9600);//iniciailzamos la comunicación
    pinMode(Trigger, OUTPUT); //pin como salida
    pinMode(Echo, INPUT);  //pin como entrada
    digitalWrite(Trigger, LOW);//Inicializamos el pin con 0
  
  
    dht.begin();
    Serial.begin(9600);
    while (!Serial) ; // Retraso para el Leonardo
    Ethernet.begin(mac, ip);
    server.begin();  // Inicia el servidor web
    Serial.print("Serv Web en la direccion: ");
    Serial.println(Ethernet.localIP());
  }
  
  
  
  void loop() {
    value = digitalRead(reed);
  
    if (value == LOW) {
      tone(buzzer, tono1);
      delay(200);
      tone(buzzer, tono2);
      lcd.clear();
      lcd.print("NOS ROBAN");
      delay(2000);
      lcd.clear();
      lcd.print("La querendona");
    } else {
      noTone(buzzer);
    }
  
    delay(200);
    EthernetClient client = server.available();
    if (client) {
      Serial.println("nuevo cliente");
      boolean currentLineIsBlank = true;
      while (client.connected()) {
        if (client.available()) {
          char c = client.read();
          Serial.write(c);
          if (c == '\n' && currentLineIsBlank)
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println("Refresh: 5");            // Actualizar cada 5 segs
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            Calculos();
            Serial.println(temperatura);
            Serial.println(humedad);
            // Desde aquí código HTML
            client.print("<head><title> Situacion del lugar</title></head>");
            client.print("<body><h1> Situacion del Ambiente</h1><p>Temperatura - ");
            client.print(temperatura);
            client.print(" grados Celsius</p>");
            client.print("<p>Humedad -  ");
            client.print(humedad); // Aqui va la humedad
            client.print(" porciento</p>");
            client.print(" Detector de incendios - ");
            client.print(llama);
            client.print("<p>Distancia - ");
            client.print(d);
            client.print(" cm</p>");
            client.println("<br />");
            client.println("<a href=\"/?button12on\"\"> Encender El Otro LED</a> ");
            client.println(" | | | ");
            client.println("<a href=\"/?button12off\"\"> Apagar El Otro LEd</a><br /> ");
            client.println("<br />");
  
  
            if (llama <= 20) {
              client.print("<p><font color='red'><h1>NOS QUEMAMOS</h1></font>");
              tone(buzzer, tono1);
              delay(200);
              tone(buzzer, tono2);
            } else {
              noTone(buzzer);
            }
  
            client.print("Personas ingresadas:");
            client.print(persona);
  
            client.print("<p><em> La pagina se actualiza cada 5 segundos.</em></p></body></html>");
            break;
          }
  
  
  
          if (readString.indexOf("?button12on") > 0) {
            digitalWrite(12, HIGH);
          }
          if (readString.indexOf("?button12off") > 0) {
            digitalWrite(12, LOW);
          }
  
  
  
          if (c == '\n')
            currentLineIsBlank = true;
          else if (c != '\r')
            currentLineIsBlank = false;
        }
      }
      delay(10);
      client.stop();
      Serial.println("cliente desconectado");
  
  
      if (d <= 7) {
        lcd.clear();
        lcd.print("Bienvenido");
        delay(2000);
        lcd.clear();
        lcd.print("La querendona");
        persona = persona + 1;
      }
      if (d <= 7) {
        microservo.write(0);
        delay(1000);
        microservo.write(40);
      }
  
  
    }
  }
  
  
  void Calculos() {
  
    humedad = dht.readHumidity();
    temperatura = dht.readTemperature();
  
    digitalWrite(Trigger, HIGH);
    delayMicroseconds(10);          //Enviamos un pulso de 10us
    digitalWrite(Trigger, LOW);
  
    t = pulseIn(Echo, HIGH); //obtenemos el ancho del pulso
    d = t / 59;           //escalamos el tiempo a una distancia en cm
  
  
  
    //Obtener la temperatura y humedad con el DHT11
    llama = analogRead(A1);
  
    // Validar que se pudo leer el sensor
    if (isnan(humedad) || isnan(temperatura)) {
      Serial.println("Fallo la lectura del DHT11");
      return;
    }
    Serial.println(llama);
  
  
    if (readString.indexOf("?button2on") > 0) {
      digitalWrite(0, HIGH);
    }
    if (readString.indexOf("?button2off") > 0) {
      digitalWrite(0, LOW);
    }
  
    if (readString.indexOf("?button3on") > 0) {
      digitalWrite(1, HIGH);
    }
    if (readString.indexOf("?button3off") > 0) {
      digitalWrite(1, LOW);
    }
  
    delay(1000);
  }
