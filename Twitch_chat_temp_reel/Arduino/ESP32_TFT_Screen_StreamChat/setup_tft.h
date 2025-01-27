// setup_tft.h
#ifndef SETUP_TFT_H
#define SETUP_TFT_H

#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <TFT_eSPI.h>

// Déclaration de l'objet TFT
extern TFT_eSPI tft;

// Fonction pour initialiser l'écran TFT et la carte SD
void setupTFT() {
    // Réglage des broches de sélection de puce (CS) à HIGH pour éviter les conflits de bus
    digitalWrite(22, HIGH); // Sélection de la puce du contrôleur tactile (si utilisé)
    digitalWrite(15, HIGH); // Sélection de la puce de l'écran TFT
    digitalWrite( 5, HIGH); // Sélection de la puce de la carte SD (doit utiliser GPIO 5 sur ESP32)

    tft.begin();  // Initialisation de l'écran TFT

    // Initialisation de la carte SD
    if (!SD.begin(5, tft.getSPIinstance())) {
        Serial.println("Card Mount Failed");  // Message d'erreur si la carte SD n'est pas montée
        return;
    }

    uint8_t cardType = SD.cardType();  // Récupération du type de carte SD

    // Vérification si aucune carte SD n'est détectée
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");  // Message d'erreur si aucune carte SD n'est détectée
        return;
    }

    // Affichage du type de carte SD
    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");  // MultiMediaCard
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");  // Carte SD standard (jusqu'à 2 Go)
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");  // Carte SD haute capacité (2 Go à 32 Go)
    } else {
        Serial.println("UNKNOWN");  // Type de carte inconnu
    }

    // Calcul et affichage de la taille de la carte SD
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);  // Taille en Mo
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    Serial.println("initialisation done.");  // Confirmation de la fin de l'initialisation
}

#endif
