#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

/* Lit une ligne de texte en toute sécurité (mieux que scanf pour du texte) */
void lireTexte(char *destination, int taille) {
    fgets(destination, taille, stdin);
    /* fgets garde le retour à la ligne '\n' : on le supprime */
    destination[strcspn(destination, "\n")] = '\0';
}

/* Lit un nombre entier */
int lireEntier(void) {
    char ligne[20];
    int nombre = 0;
    lireTexte(ligne, sizeof(ligne));
    sscanf(ligne, "%d", &nombre);
    return nombre;
}

/* Convertit un texte en minuscules (pour une recherche insensible à la casse) */
void enMinuscules(char *texte) {
    for (int i = 0; texte[i] != '\0'; i++) {
        texte[i] = tolower((unsigned char)texte[i]);
    }
}
