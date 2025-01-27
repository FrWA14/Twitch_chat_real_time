// jpeg_fnction_TFT.h
#ifndef JPEG_FONCTION_H
#define JPEG_FONCTION_H

#include <JPEGDecoder.h>
//#include <TFT_eSPI.h>

// Déclaration de l'objet TFT
//extern TFT_eSPI tft;

// Fonction pour dessiner un fichier JPEG depuis la carte SD sur l'écran TFT
void drawSdJpeg(const char *filename, int xpos, int ypos) {
    // Ouverture du fichier JPEG sur la carte SD
    File jpegFile = SD.open(filename, FILE_READ);

    // Vérification si le fichier a été ouvert avec succès
    if (!jpegFile) {
        //Serial.print("ERROR: File \""); Serial.print(filename); Serial.println("\" not found!");
        return;  // Si le fichier n'est pas trouvé, on sort de la fonction
    }

    //Serial.println("===========================");
    //Serial.print("Drawing file: "); Serial.println(filename);
    //Serial.println("===========================");

    // Décodage du fichier JPEG en utilisant la bibliothèque JPEGDecoder
    bool decoded = JpegDec.decodeSdFile(jpegFile);

    // Si le fichier JPEG est correctement décodé, il est rendu sur l'écran
    if (decoded) {
        jpegRender(xpos, ypos);  // Appel de la fonction pour afficher l'image sur l'écran
    } else {
        Serial.println("Jpeg file format not supported!");  // Message d'erreur si le format JPEG n'est pas supporté
    }
}

#endif
