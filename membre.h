#ifndef MEMBRE_H
#define MEMBRE_H

#include "bibliotheque.h"   /* pour le type "Resultat" */

#define MAX_MEMBRES 50  /* nombre maximum de membres */
#define TAILLE_NOM 100  /* taille max d'un nom */

typedef struct {
    int id;
    char nom[TAILLE_NOM];
} Membre;

/* Fonctions "console" (appelees par le menu texte) */
void chargerMembres(void);
void ajouterMembre(void);
void afficherMembres(void);
void supprimerMembre(void);

/* Fonctions "logique" (ne lisent rien au clavier, n'affichent rien) */
Resultat creerMembre(const char *nom, int *idCree);
Resultat retirerMembre(int id);
int membreExiste(int id);            /* 1 si le membre existe, 0 sinon */
const char *nomMembre(int id);       /* nom du membre, ou "Inconnu" */
int nombreDeMembres(void);
const Membre *membreNumero(int position);

#endif
