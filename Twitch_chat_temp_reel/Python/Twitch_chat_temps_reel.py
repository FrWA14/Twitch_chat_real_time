import socket
import requests
import re
import threading

# Informations de ton application
client_id = "client_id"
client_secret = "client_secret"
twitch_channel = "nom_chaine_twitch"    #nom de la chaine à retourné le chat en temps réel
                                                                    #le nom de la chaine correspond en minuscule apres https://www.twitch.tv/nom_chaine_twitch

class TwitchChatBot:
    def __init__(self, client_id, client_secret, channel):
        self.client_id = client_id
        self.client_secret = client_secret
        self.channel = channel
        self.oauth_token = None
        self.socket = None

    def get_oauth_token(self):
        """Obtenir un token OAuth via client_credentials."""
        print("[INFO] Obtention du token OAuth...")
        url = "https://id.twitch.tv/oauth2/token"
        data = {
            'client_id': self.client_id,
            'client_secret': self.client_secret,
            'grant_type': 'client_credentials'
        }

        response = requests.post(url, data=data)
        response.raise_for_status()
        self.oauth_token = response.json()['access_token']
        print(f"[SUCCESS] Token OAuth reçu : {self.oauth_token}")

    def connect_to_chat(self):
        """Se connecter au chat Twitch via IRC."""
        print("[INFO] Connexion au serveur de chat Twitch...")
        server = "irc.chat.twitch.tv"
        port = 6667
        self.socket = socket.socket()
        try:
            self.socket.connect((server, port))
            print("[SUCCESS] Connexion au serveur IRC réussie.")
            
            # Envoi des informations d'authentification
            print("[INFO] Envoi des informations d'authentification...")
            self.socket.sendall(f"PASS oauth:{self.oauth_token}\n".encode("utf-8"))
            self.socket.sendall(f"NICK justinfan12345\n".encode("utf-8"))  # Utiliser un pseudo générique "justinfan12345"
            self.socket.sendall(f"JOIN #{self.channel}\n".encode("utf-8"))
            print("[SUCCESS] Authentification envoyée, connexion au chat en cours...")
        except Exception as e:
            print(f"[ERROR] Impossible de se connecter au serveur IRC : {e}")
            raise

    def listen_to_chat(self):
        """Écouter les messages du chat."""
        print("[INFO] Démarrage de l'écoute du chat...")
        while True:
            try:
                response = self.socket.recv(2048).decode("utf-8")
                if response.startswith("PING"):
                    # Répondre au PING pour garder la connexion ouverte
                    print("[PING] Réponse envoyée au serveur.")
                    self.socket.sendall("PONG :tmi.twitch.tv\n".encode("utf-8"))
                else:
                    # Extraire et afficher les messages
                    messages = re.findall(r":(.*?)!.*?@.*?\.tmi\.twitch\.tv PRIVMSG #.*? :(.*)", response)
                    for user, message in messages:
                        print(f"[CHAT] {user}: {message}")
            except Exception as e:
                print(f"[ERROR] Erreur lors de la réception des messages : {e}")
                break

    def start(self):
        """Démarrer le bot."""
        try:
            self.get_oauth_token()
            self.connect_to_chat()
            listener_thread = threading.Thread(target=self.listen_to_chat)
            listener_thread.start()
        except Exception as e:
            print(f"[ERROR] Une erreur est survenue : {e}")

# Lancer le bot
if __name__ == "__main__":
    bot = TwitchChatBot(client_id, client_secret, twitch_channel)
    bot.start()
