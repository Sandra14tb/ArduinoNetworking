  #include <SPI.h>   // El shield Ethernet usa SPI
  #include <Ethernet.h> //Libreria de Ethernet
  #include "DHT.h" //Libreria del sensor
  #include <LiquidCrystal.h> //Libreria del LCD
  #include <Servo.h> //Libreria del ServoMotor
  
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; //Mac Address
  IPAddress ip(192, 168, 137, 5); // IP estática
  EthernetServer server(80); //Puerto
  Servo microservo; //Declarar Servo
  
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
  volatile int value = HIGH; //Valor del reed Switch
  const int tono1 = 1000; //Tonos del buzzer
  const int tono2 = 1250;
  int persona = 0; //Contador para las personas que ingresan a la tienda
  
  
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
  
  
    Serial.begin(9600);//inicializamos la comunicación
    pinMode(Trigger, OUTPUT); //pin como salida
    pinMode(Echo, INPUT);  //pin como entrada
    digitalWrite(Trigger, LOW);//Inicializamos el pin con 0
  
  
    dht.begin(); //Inicializar el dht
    Serial.begin(9600);
    while (!Serial) ; // Retraso para el Leonardo
    Ethernet.begin(mac, ip);
    server.begin();  // Inicia el servidor web
    Serial.print("Serv Web en la direccion: ");
    Serial.println(Ethernet.localIP());
  }
  
  
  
  void loop() {
    //Calcular el valor del reed switch
    value = digitalRead(reed);
  
    if (value == LOW) { //Si se detecta algo por el reed switch, entonces suena el buzzer y se muestra en el LCD mensaje de alerta
      tone(buzzer, tono1);
      delay(200);
      tone(buzzer, tono2);
      lcd.clear(); //Limpiar LCD
      lcd.print("NOS ROBAN"); //Imprimir mensaje en LCD
      delay(2000);
      lcd.clear(); //Regresar al nombre de la Tienda
      lcd.print("La querendona");
    } else {
      noTone(buzzer);
    }
  
    delay(200);
    EthernetClient client = server.available(); // Comprobamos si hay peticiones
    if (client) {    // En caso afirmativo
      Serial.println("nuevo cliente");
      boolean currentLineIsBlank = true;
      while (client.connected()) {
        if (client.available()) { // Hay algo pendiente de leer
          char c = client.read(); // Leemos los caracteres de uno en uno
          Serial.write(c);
          if (c == '\n' && currentLineIsBlank)
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println("Refresh: 5");            // Actualizar cada 5 segs
            client.println();
            client.println("<!DOCTYPE HTML>");   // Envia la pagina Web
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
  
             //Si se detecta que llama es menor igual a 20 entonces manda mensaje de alerta en la página y suena el buzzer
            if (llama <= 20) {
              client.print("<p><font color='red'><h1>NOS QUEMAMOS</h1></font>"); //Mostrar alerta
              tone(buzzer, tono1); //Sonar buzzer
              delay(200);
              tone(buzzer, tono2);
            } else {
              noTone(buzzer);
            }
  
            client.print("Personas ingresadas:"); //Imprimir el contador de las personas que ingresan a la tienda
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
      client.stop(); // Cerrar conexion
      Serial.println("cliente desconectado");
  
      //Si se detecta que esta a menos de 7cm entonces imprime bienvenido en el LCD y se incrementa el contador
      if (d <= 7) {
        lcd.clear();
        lcd.print("Bienvenido"); //Mensaje para cuando alguien ingresa
        delay(2000);
        lcd.clear();
        lcd.print("La querendona");
        persona = persona + 1; //Incrementar contador
      }
      if (d <= 7) { //Movimiento del servo de ofertas
        microservo.write(0);
        delay(1000);
        microservo.write(40);
      }
  
  
    }
  }
  
  //metodo para calcular la temperatura, humedad del DTH11
  void Calculos() {
  
    humedad = dht.readHumidity();//Calcular Humedad
    temperatura = dht.readTemperature();//Calcular Temperatura
  
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
  
    delay(1000);
  }
