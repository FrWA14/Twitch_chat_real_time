#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "setup_tft.h"
#include "jpeg_fonction_tft.h"

TFT_eSPI tft = TFT_eSPI();

// Informations de connexion Wi-Fi
const char *ssid = "TP-nom_box_wifi";
const char *password = "mot_de_passe_box_wifi";

// Informations Twitch
const char *client_id = "client_id";
const char *client_secret = "client_secret";
const char *twitch_channel = "twitch_channel"; //chaine à lire le tchat

String oauth_token; // Le token OAuth sera stocké ici
WiFiClientSecure httpsClient; // Pour les connexions HTTPS
WiFiClient client;            // Pour la connexion IRC Twitch

#define MAX_MESSAGES 24  // Nombre maximum de messages à afficher
String messages[MAX_MESSAGES];  // Tableau pour stocker les messages
int messageCount = 0;  // Nombre de messages actuels

// Déclaration des constantes pour les boutons
const int BUTTON_RESET = 22;
const int BUTTON_SAVE = 33;
// Déclaration des constantes pour les LED
const int LED_BLEU = 12;
const int LED_JAUNE = 13;
const int LED_ROUGE = 14;

void connectToWiFi() {
  Serial.print("[INFO] Connexion au Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\n[SUCCESS] Connecté au Wi-Fi !");
}

void getOAuthToken() {
  const char *host = "id.twitch.tv";
  const int httpsPort = 443;

  if (!httpsClient.connect(host, httpsPort)) {
    Serial.println("[ERROR] Échec de connexion au serveur Twitch API.");
    return;
  }

  Serial.println("[INFO] Connexion à Twitch API réussie.");

  // Corps de la requête POST
  String postData = "client_id=" + String(client_id) +
                    "&client_secret=" + String(client_secret) +
                    "&grant_type=client_credentials";

  // Construction de la requête HTTP
  httpsClient.println("POST /oauth2/token HTTP/1.1");
  httpsClient.println("Host: id.twitch.tv");
  httpsClient.println("Content-Type: application/x-www-form-urlencoded");
  httpsClient.print("Content-Length: ");
  httpsClient.println(postData.length());
  httpsClient.println("Connection: close");
  httpsClient.println();
  httpsClient.println(postData);

  // Lecture de la réponse
  String response;
  while (httpsClient.connected() || httpsClient.available()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") break; // Fin des headers
  }

  // Lecture du corps de la réponse
  response = httpsClient.readString();
  Serial.println("[INFO] Réponse de l'API :");
  Serial.println(response);

  // Extraire le token de la réponse avec ArduinoJson
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, response);
  oauth_token = doc["access_token"].as<String>();
  Serial.println("[SUCCESS] Token OAuth reçu : " + oauth_token);
}

void connectToTwitch() {
  const char *server = "irc.chat.twitch.tv";
  const int port = 6667;

  if (client.connect(server, port)) {
    Serial.println("[SUCCESS] Connecté au serveur IRC Twitch.");

    // Envoi des informations d'authentification
    client.println("PASS oauth:" + oauth_token);
    client.println("NICK justinfan12345"); // Un pseudo générique
    client.println("JOIN #" + String(twitch_channel));
    Serial.println("[INFO] Connexion au chat Twitch envoyée.");
  } else {
    Serial.println("[ERROR] Impossible de se connecter au serveur IRC Twitch.");
  }
}

// Fonction pour ajouter un message au tableau et mettre à jour l'écran
void addMessageToDisplay(String newMessage) {
  // Ajouter le message au tableau
  if (messageCount < MAX_MESSAGES) {
    messages[messageCount++] = newMessage;
  } else {
    // Décaler les messages si la liste est pleine
    for (int i = 1; i < MAX_MESSAGES; i++) {
      messages[i - 1] = messages[i];
    }
    messages[MAX_MESSAGES - 1] = newMessage;
  }

  // Effacer l'écran et réafficher les messages
  tft.fillScreen(TFT_BLACK);
  int y = tft.height(); // Commence en bas de l'écran
  for (int i = messageCount - 1; i >= 0; i--) {
    y -= 20; // Taille d'une ligne (à ajuster selon la taille de la police)
    if (y < 0) break; // Arrête si on dépasse le haut de l'écran
    tft.setCursor(0, y);
    tft.print(messages[i]);
  }
}

// Fonction pour écouter le chat Twitch
void listenToChat() {
  if (client.connected() && client.available()) {
    String line = client.readStringUntil('\n');
    line.trim();

    if (line.startsWith("PING")) {
      client.print("PONG :tmi.twitch.tv\r\n");
      Serial.println("[PING] Réponse envoyée.");
    } else if (line.indexOf("PRIVMSG") != -1) {
      int userStart = line.indexOf(":") + 1;
      int userEnd = line.indexOf("!");
      String username = line.substring(userStart, userEnd);

      int messageStart = line.indexOf(":", userEnd) + 1;
      String message = line.substring(messageStart);

      // Afficher le message dans le moniteur série
      Serial.println("[CHAT] " + username + ": " + message);

      // Ajouter le message formaté à l'écran
      addMessageToDisplay(username + ": " + message);
    }
  }
}


void setup() {
  // Initialisation du port série
  Serial.begin(115200);
    // Configuration des entrées/sorties
  pinMode(BUTTON_RESET, INPUT);
  pinMode(BUTTON_SAVE, INPUT);
  pinMode(LED_BLEU, OUTPUT);
  pinMode(LED_JAUNE, OUTPUT);
  pinMode(LED_ROUGE, OUTPUT);

  setupTFT();

  // Affichage d'une image d'initialisation
  tft.setRotation(2);  // 1:paysage //
  tft.fillScreen(0xFFFF); //Fond d'écrant en blanc
  //position de l'image sur l'écran
  int x = (tft.width()  - 300) / 2 - 1;
  int y = (tft.height() - 300) / 2 - 1;
  //affichage de l'image
  drawSdJpeg("/PP.jpg", x, y); // Affiche une image JPEG depuis la carte SD
  delay(2000);
  tft.fillScreen(0x0000); //Fond d'écrant en noir
  tft.setTextColor(0xFFFF, 0x0000);  // Texte blanc sur fond noir

  // Connexion au Wi-Fi
  connectToWiFi();
  // Obtenir le token OAuth
  getOAuthToken();
  // Connexion au serveur Twitch IRC
  connectToTwitch();
}

void loop() {
  // Écouter et afficher les messages du chat
  listenToChat();

  // Vérifier si la connexion est toujours active
  if (!client.connected()) {
    Serial.println("[INFO] Reconnexion au serveur Twitch...");
    connectToTwitch();
  }

  delay(10); // Laisser du temps pour éviter la surcharge CPU
}

