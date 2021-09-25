#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define PIN_TRIG D5
#define PIN_ECHO D6
#define SLEEP_TIME 3600e6                    // sleep intervalls in us
//#define SLEEP_TIME 60*1*1000000                    // minuto o 60 segundos, para 1 hora 60*60*1000000
//#define SLEEP_TIME 60*1*1000000                    // minuto o 60 segundos, para 1 hora 60*60*1000000

// const char* ssid     = "TP-LINK_B440";
// const char* password = "29407828";

const char* ssid     = "NuCom_0201A0_plus";
const char* password = "VYJ81Sahr961";

const char *mqtt_server = "trajano.es";
const int mqtt_port = 1883;
const char *mqtt_user = "web_client"; //el cliente web, este esta en las tablas de sql
const char *mqtt_pass = "121212";

WiFiClient espClient;
PubSubClient client(espClient); //pasamos la instancia, para hacer todo a traves de esta conexion

long lastMsg = 0; //para guardar el ultimo mensaje
char msg[25]; //para enviar el mensaje en tipo array, no se puede enviar un string asi nada mas



//FIXME: cambiar para poner el sensor de distancia
int distancia = 0; // pruebas
//int temp2 = 1;
//int volts = 2;

//*****************************
//*** DECLARACION FUNCIONES ***
//*****************************
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
int medicion();


void setup() {
	pinMode(2, OUTPUT);
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
	pinMode(BUILTIN_LED, OUTPUT);
	Serial.begin(115200);
	randomSeed(micros()); //para semillas para generar valores aleatorios
	setup_wifi();
	client.setServer(mqtt_server, mqtt_port); //pasamos parametros para que se conecte
	client.setCallback(callback); // funcion para que cada vez que llegue un mensaje
}

void loop() {
	if (!client.connected()) { //nos conectamos a MQTT
		reconnect(); //reconectamos a MQTT
	}

	client.loop(); //metodo indispensable, no tengo que poner ningun delay despues de este metodo

	long now = millis();
	if (now - lastMsg > 500){
		lastMsg = now;
		//distancia++;
		
		distancia = medicion();
		distancia = 400 - distancia;
		//Serial.println(distancia); 

		//temp2++;
		//volts++;

		//String to_send = String(distancia) + "," + String(temp2) + "," + String(volts);
		String to_send = String(distancia);
		to_send.toCharArray(msg, 25);   
		Serial.print("Publicamos mensaje -> ");
		Serial.println(msg);
		client.publish("values", msg);
		

		delay(2000);
		//entramos en deepSleep()
  		ESP.deepSleep(SLEEP_TIME);
	}

	//entramos en deepSleep()
  	// ESP.deepSleep(SLEEP_TIME);
}
//*****************************
//*** FUNCION QUE REALIZA LA MEDICION DE NIVEL DE AGUA ***
//*****************************

int medicion(){


  digitalWrite(PIN_TRIG, LOW);  //para generar un pulso limpio ponemos a LOW 4us
  delayMicroseconds(2);
   
  digitalWrite(PIN_TRIG, HIGH);  //generamos Trigger (disparo) de 10us
  delayMicroseconds(15);
  digitalWrite(PIN_TRIG, LOW);
   
  //tiempo = pulseIn(PIN_ECHO, HIGH);
  int distance = pulseIn(PIN_ECHO, HIGH, 26000);
  distancia = distance/58.3;

  //Serial.println(distancia);

  delay(2000);

  //distancia = 44;  

  return distancia;
  
}


//*****************************
//***    CONEXION WIFI      ***
//*****************************
void setup_wifi(){
	delay(10);
	// Nos conectamos a nuestra red Wifi
	Serial.println();
	Serial.print("Conectando a ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("Conectado a red WiFi!");
	Serial.println("Dirección IP: ");
	Serial.println(WiFi.localIP());
}

//*****************************
//***    CALLBACK PARA ESCUCHAR LO QUE LLEGA DEL BROKER      ***
//*****************************

void callback(char* topic, byte* payload, unsigned int length){ //recibe desde la libreria, topico, payload, tamano de mensaje
	String incoming = ""; //para almacenar lo que llega desde el broker
	Serial.print("Mensaje recibido desde -> ");
	Serial.print(topic);
	Serial.println("");
	for (int i = 0; i < length; i++) { //for para recibir el mensaje, porque es un array de bytes no un string, por eso usamos el for
		incoming += (char)payload[i];
	}
	incoming.trim(); //quito los espacios o el ENTER de la parte final \n\s
	Serial.println("Mensaje -> " + incoming);

	if ( incoming == "on") {
		digitalWrite(BUILTIN_LED, HIGH);
	} else {
		digitalWrite(BUILTIN_LED, LOW);
	}
}
//*****************************
//***    RECONECCION MQTT   ***
//*****************************

void reconnect() {

	while (!client.connected()) {
		Serial.print("Intentando conexión Mqtt...");
		// Creamos un cliente ID
		String clientId = "esp32_"; 
		clientId += String(random(0xffff), HEX); //agregamos un numero random para que sea un tipo distinto de clientId
		// Intentamos conectar
		if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) { //si se conecta hace lo de adentro
			Serial.println("Conectado!");
			// Nos suscribimos
			client.subscribe("led1");
			client.subscribe("led2");
		} else { //sino
			Serial.print("falló :( con error -> ");
			Serial.print(client.state());
			Serial.println(" Intentamos de nuevo en 5 segundos");

			delay(5000);
		}
	}
}
