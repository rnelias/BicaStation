
#include <SoftwareSerial.h>
#include <String.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;

// --- Constantes ---
const float pi = 3.14159265;     // Número de pi
int period = 5000;               // Tempo de medida(miliseconds)
int delaytime = 2000;            // Invervalo entre as amostras (miliseconds)
int radius = 147;                // Raio do anemometro(mm)

// --- Variáveis Globais ---
unsigned int Sample  = 0;        // Armazena o número de amostras
unsigned int counter = 0;        // Contador para o sensor  
unsigned int RPM = 0;            // Rotações por minuto
float windspeed = 0.0;           // Velocidade do vento (km/h)
float temp = 0.0;                // Temperatura em Celsius
float umid = 0.0;                // Umidade relativa %
float pres = 0.0;                // Pressao em Pa
int pin =0;
float valor =0;
int Winddir =0;
int idir  = 0;
char* dir[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW" };  // human readable wind directions are only shown on the OLED



// --- Modulo GSM ---

SoftwareSerial mySerial(10, 11);
String apn;                      // Endereco da operadora
String apn_username;             // Nome de usuário para login na operadora
String apn_passwd;               // Senha para login na operadora

void setup()
{
  mySerial.begin(9600);               // the GPRS baud rate   
  Serial.begin(9600);    // the GPRS baud rate 
//  pinMode(3,INPUT);
//  pinMode(4,INPUT);
//  pinMode(5,INPUT);  
//  pinMode(6,INPUT);  
  delay(1000);

  // Anemometro
  pinMode(2,INPUT);
  digitalWrite(2, HIGH);    //internall pull-up active

  // temperatura, umidade e pressao
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  
}
 
void loop()
{
      // anemometro
      Sample++;
      Serial.print(Sample);
      Serial.print(": Starting measurement... \n");

      // ANEMOMETRO
      windvelocity();
      Serial.print("Counter: ");
      Serial.print(counter);
      Serial.print(";  RPM: ");
      RPMcalc();
      Serial.print(RPM);
      Serial.print(";  Wind speed: ");

      // print knots
      WindSpeed_KNOT();
      Serial.print(windspeed);
      Serial.print(" [knot]; ");  

      // BIRUTA (DIRECAO DO VENTO)
      WindDirection();
      Serial.print(" Wind direction: ");
      Serial.print(Winddir);
      Serial.print(" graus ("+String(dir[idir])+") ["+String(valor)+"v] \n");

      // BM280 (TEMPERATURA, UMIDADE E PRESSAO)
      ReadBME280();

      Serial.println("   finished.\n");

      //delay (1000);

      //SendDataToThinkSpeak(); // ENVIO DOS DADOS PARA O THINGSPEAK
   
      //if (mySerial.available())
      //Serial.write(mySerial.read());
      //delay(delaytime);  //taxa de atualização  
      //ShowSerialData();
        
}

//===============================================================================
void SendDataToThinkSpeak()
{
  Serial.print(" sending data to ThingSpeak... ");  
  mySerial.println("AT");
  delay(1000);
  mySerial.println("AT+CPIN?");
  delay(1000);
  mySerial.println("AT+CREG?");
  delay(1000);
  mySerial.println("AT+CGATT?");
  delay(1000);
  mySerial.println("AT+CIPSHUT");
  delay(1000);
  mySerial.println("AT+CIPSTATUS");
  delay(2000);
  mySerial.println("AT+CIPMUX=0");
  delay(2000);
  apn = "claro.com.br";
  apn_username = "claro";
  apn_passwd = "claro";
  mySerial.println("AT+CSTT=\"" + apn + "\",\"" + apn_username + "\",\"" + apn_passwd +  "\"\n"); //start task and setting the APN,
  delay(1000);
  mySerial.println("AT+CIICR"); //bring up wireless connection
  delay(3000);
  mySerial.println("AT+CIFSR"); //get local IP adress
  delay(2000);
  mySerial.println("AT+CIPSPRT=0");
  delay(3000);
  mySerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\""); //start up the connection
  delay(6000);
  mySerial.println("AT+CIPSEND"); //begin send data to remote server
  delay(4000);
  String str="GET https://api.thingspeak.com/update?api_key=F4WSQ1YCLM8SB0RT&field1="+String(windspeed)+"&field2="+String(Winddir)+"&field3="+String(temp);
  mySerial.println(str);//begin send data to remote server
  delay(4000);
  mySerial.println((char)26);//sending
  delay(5000);//waitting for reply, important! the time is base on the condition of internet 
  mySerial.println();
  mySerial.println("AT+CIPSHUT");//close the connection
  delay(100);
} 

//===============================================================================
void ShowSerialData()
{
  while(mySerial.available()!=0)
    Serial.write(mySerial.read());
}

//===============================================================================
//                   Função para medir velocidade do vento
//===============================================================================
void windvelocity()
{
  windspeed = 0;
  
  counter = 0;  
  attachInterrupt(0, addcount, RISING);
  unsigned long millis();       
  long startTime = millis();
  while(millis() < startTime + period) {}
}

//===============================================================================
//                     Função para calcular o RPM
//===============================================================================
void RPMcalc()
{
  RPM=((counter)*60)/(period/1000);  // Calculate revolutions per minute (RPM)
}

//===============================================================================
//                      Velocidade do vento em m/s
//===============================================================================
void WindSpeed_MPS()
{
  windspeed = ((4 * pi * radius * RPM)/60) / 1000;  //Calcula a velocidade do vento em m/s
 
}

//===============================================================================
//                      Velocidade do vento em km/h
//===============================================================================
void WindSpeed_KPH()
{
  windspeed = (((4 * pi * radius * RPM)/60) / 1000)*3.6;  //Calcula velocidade do vento em km/h
 
}

//===============================================================================
//                        Velocidade do vento em nos
//===============================================================================
void WindSpeed_KNOT()
{
  windspeed = (((4 * pi * radius * RPM)/60) / 1000)*1.94;  //Calcula velocidade do vento em nós
 
}


//===============================================================================
//                            direcao do vento
//===============================================================================
void WindDirection()
{
    valor = analogRead(pin)* (5.0 / 1023.0);
     
//    Serial.print("\n leitura do sensor :");
//    Serial.print(valor);
//    Serial.println(" volt");

    if (valor <= 2.33) {
    Winddir = 0;
    idir = 0;
    }
    else if (valor <= 2.56) { 
    Winddir = 315;
    idir = 1;    
    }
    else if (valor <= 2.64) { 
    Winddir = 270;
    idir = 2;    
    }
    else if (valor <= 2.90) { 
    Winddir = 225;
    idir = 3;    
    }
    else if (valor <= 3.20) { 
    Winddir = 180;
    idir = 4;    
    }
    else if (valor <= 3.57) { 
    Winddir = 135;
    idir = 5;
    }
    else if (valor <= 4.15) {  
    Winddir = 90;
    idir = 6;    
    }
    else if (valor <= 5.00) {  
    Winddir = 45;
    idir = 7;
    }
    
}

//===============================================================================
//                            Incrementa contador
//===============================================================================
void addcount()
{
  counter++;
} 

//===============================================================================
//                       Lê sensor de temperatura, umidade e pressão
//===============================================================================
void ReadBME280()
{
    Serial.print("Temperature = ");
    temp = bme.readTemperature();
    Serial.print(temp);
    Serial.print("*C; ");

    Serial.print("Pressure = ");
    pres = bme.readPressure() / 100.0F;
    Serial.print(pres);
    Serial.print("hPa; ");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.print("m; ");

    Serial.print("Humidity = ");
    umid = bme.readHumidity();
    Serial.print(umid);
    Serial.print("%\n ");

}
