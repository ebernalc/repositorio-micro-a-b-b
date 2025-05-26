//Bernal, Álvarez y Barboza.
//Sistema de control de acceso con RFID/NFC.
#include <SPI.h> //Libreria del master-slave, para el CLK, MOSI, MISO y SS.
#include <MFRC522.h> //Libreria del lector NFC.
#include <EEPROM.h> //Libreria de la memoria EEPROM, para guardar los accesos. 

#define RST_PIN         22 //Pin del reset.
#define SS_PIN          5  //Pin del slave select (SDA en el RC522).
#define RELAY_PIN        2  //Pin IN del relé.
#define EEPROM_SIZE     10 //Tamaño de memoria 10 bytes. 

MFRC522 mfrc522(SS_PIN, RST_PIN); //Constructor de clase definido en la libreria, para inicializar el objeto con los valores necesarios. 

//Matriz de UID de tarjetas registradas, cada UID está compuesto por 4 bytes en formato hexadecimal.
byte registrotarjetas[10][4] = {
  {0xAC, 0x30, 0x1F, 0xF4},
  {0xCC, 0xFA, 0x85, 0xF3},
  {0x8C, 0xCB, 0xF6, 0xF3},
  {0x3C, 0xB2, 0xDE, 0xF3},
  {0x4C, 0x2E, 0xD1, 0xF3},
  {0x1C, 0x79, 0xFB, 0xF3},
  {0xFC, 0x47, 0xFF, 0xF3},
  {0xCC, 0xEA, 0xEF, 0xF3},
  {0xAC, 0xE6, 0x85, 0xF3},
  {0x5C, 0xA4, 0xDD, 0xF3}
};

//Nombres por defecto colocados a las tarjetas.
String cardNames[10] = {
  "Tarjeta 1", "Tarjeta 2", "Tarjeta 3", "Tarjeta 4", "Tarjeta 5",
  "Tarjeta 6", "Tarjeta 7", "Tarjeta 8", "Tarjeta 9", "Tarjeta 10"
};

//Array de booleanos, lleva el registro de permisos y se guarda en la EEPROM. 
bool accessGranted[10];

//Interfaz del menú principal.
void mostrarMenu() {
  Serial.println(F("\n-------------------Gestión accesos------------------"));
  Serial.println(F("1. Permitir acceso."));
  Serial.println(F("2. Bloquear acceso."));
  Serial.println(F("3. Ver estado de todas las tarjetas."));
  Serial.println(F("---------------------------------------------------"));
}

//Función para gestionar los comandos recibidos en el puerto serial.
void gestionarSerial() {
  if (Serial.available()) {
    String entrada = Serial.readStringUntil('\n'); //Se coloca esto para que no de error al dar enter después de colocar un comando en el monitor serial.
    entrada.trim();
    char opcion = entrada.charAt(0);

    if (opcion == '1' || opcion == '2') {
      bool permitir = (opcion == '1');
      Serial.println(permitir ? "Seleccionado: Permitir acceso" : "Seleccionado: Bloquear acceso");
      //Imprime el estado de las tarjetas.
      Serial.println("Tarjetas registradas:");
      for (int i = 0; i < 10; i++) {
        Serial.print(i + 1);
        Serial.print(". ");
        Serial.print(cardNames[i]);
        Serial.print(" - Estado: ");
        Serial.println(accessGranted[i] ? "PERMITIDO" : "BLOQUEADO");
      }

      Serial.println("Ingrese el número de tarjeta a modificar (1-10):");
      while (!Serial.available());
      String seleccionStr = Serial.readStringUntil('\n');
      seleccionStr.trim();
      int seleccion = seleccionStr.toInt();
      while (Serial.available()) Serial.read();

      if (seleccion >= 1 && seleccion <= 10) {
        int index = seleccion - 1;
        accessGranted[index] = permitir;
        EEPROM.write(index, permitir);
        EEPROM.commit();
        Serial.print("La ");
        Serial.print(cardNames[index]);
        Serial.println(permitir ? " ha sido PERMITIDA." : " ha sido BLOQUEADA.");
      } else {
        Serial.println("Entrada inválida. Solo se aceptan números del 1 al 10.");
      }

      mostrarMenu();
    } else if (opcion == '3') {
      Serial.println("Estado actual de las tarjetas:");
      for (int i = 0; i < 10; i++) {
        Serial.print(i + 1);
        Serial.print(". ");
        Serial.print(cardNames[i]);
        Serial.print(" - Estado: ");
        Serial.println(accessGranted[i] ? "PERMITIDO" : "BLOQUEADO");
      }
      mostrarMenu();
    } else {
      Serial.println("Opción inválida. Intente nuevamente.");
      mostrarMenu();
    }
  }
}

//Función para comparar con el UID de las tarjetas registradas.
bool compararUID(byte* uid, byte size, byte* tarjetaRegistrada) {
  for (byte i = 0; i < size; i++) {
    if (uid[i] != tarjetaRegistrada[i]) {
      return false;
    }
  }
  return true;
}

void setup() {
  Serial.begin(115200);           //Tasa de baudios.
  SPI.begin(18, 19, 23, SS_PIN);  //Configuración de pines para el bus SPI.
  mfrc522.PCD_Init();             //Inicialización del lector RFID.
  pinMode(RELAY_PIN, OUTPUT);     
  digitalWrite(RELAY_PIN, HIGH);  //Relé apagado inicialmente.

  EEPROM.begin(EEPROM_SIZE);      //Inicia EEPROM.
  for (int i = 0; i < 10; i++) {
    accessGranted[i] = EEPROM.read(i);  //Lee estados guardados (permisos).
  }

  mostrarMenu();  //Muestra el menú por serial.
}

void loop() {
  gestionarSerial(); //Revisa los comandos por serial.

  //Condicional que verifica si no hay una nueva tarjeta o si no se pudo leer.
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  byte* uid = mfrc522.uid.uidByte;  //Obtiene el UID de la tarjeta.
  byte uidSize = mfrc522.uid.size;  //Obtiene el tamaño del UID.
  while (Serial.available()) Serial.read(); //Limpia el buffer serial.

  
  bool encontrada = false; 
  for (int i = 0; i < 10; i++) {
    if (compararUID(uid, uidSize, registrotarjetas[i])) { //Compara el UID de la nueva tarjeta con los registrados.
      encontrada = true;
      Serial.print("Detectada: ");
      Serial.println(cardNames[i]);
      //Si tiene acceso, enciende el relé, si no, solo imprime "Acceso denegado". 
      if (accessGranted[i]) {
        Serial.println("ACCESO CONCEDIDO");
        digitalWrite(RELAY_PIN, LOW);   
        delay(3000);
        digitalWrite(RELAY_PIN, HIGH);  
      } else {
        Serial.println("ACCESO DENEGADO");
      }
      break;
    }
  } //En caso de no encontrar el UID, imprime UID no registrado.
  if (!encontrada) {
    Serial.println("UID NO REGISTRADO");
  }

  mfrc522.PICC_HaltA();       // Detiene comunicación con tarjeta
  mfrc522.PCD_StopCrypto1();  // Reinicia el lector RFID
  mostrarMenu();              // Mostrar menú tras lectura
}

