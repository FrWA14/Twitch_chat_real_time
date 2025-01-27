#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// Informations de connexion Wi-Fi
const char *ssid = "nom_box_wifi";
const char *password = "mot_de_passe_box_wifi";

// Informations Twitch
const char *client_id = "client_id";
const char *client_secret = "client_secret";
const char *twitch_channel = "twitch_channel"; //chaine à lire le chat

String oauth_token; // Le token OAuth sera stocké ici
WiFiClientSecure httpsClient; // Pour les connexions HTTPS
WiFiClient client;            // Pour la connexion IRC Twitch

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

void listenToChat() {
  if (client.connected() && client.available()) {
    String line = client.readStringUntil('\n'); // Lire une ligne du chat
    line.trim(); // Retirer les espaces ou retours à la ligne inutiles

    if (line.startsWith("PING")) {
      // Répondre au PING pour garder la connexion ouverte
      client.print("PONG :tmi.twitch.tv\r\n");
      Serial.println("[PING] Réponse envoyée.");
    } else if (line.indexOf("PRIVMSG") != -1) {
      // Extraire le pseudo et le message du chat
      int userStart = line.indexOf(":") + 1;
      int userEnd = line.indexOf("!");
      String username = line.substring(userStart, userEnd);

      int messageStart = line.indexOf(":", userEnd) + 1;
      String message = line.substring(messageStart);

      Serial.println("[CHAT] " + username + ": " + message);
    }
  }
}

void setup() {
  // Initialisation du port série
  Serial.begin(115200);

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
