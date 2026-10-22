#include <stdio.h>
#include <string.h>
#include "membre.h"
#include "bibliotheque.h"

#include "utils.h"

static Membre membres[MAX_MEMBRES];
static int nombreMembres = 0;

static int prochainIdMembre(void) {
    int max = 0;
    for (int i = 0; i < nombreMembres; i++) {
        if (membres[i].id > max) {
            max = membres[i].id;
        }
    }
    return max + 1;
}

/* Sauvegarde dans membres.txt (une ligne par membre : id;nom) */
static void sauvegarderMembres(void) {
    FILE *f = fopen("membres.txt", "w");
    if (f == NULL) {
        printf("Erreur : impossible de sauvegarder les membres.\n");
        return;
    }
    for (int i = 0; i < nombreMembres; i++) {
        fprintf(f, "%d;%s\n", membres[i].id, membres[i].nom);
    }
    fclose(f);
}

void chargerMembres(void) {
    FILE *f = fopen("membres.txt", "r");
    if (f == NULL) {
        return;
    }
    char ligne[200];
    while (fgets(ligne, sizeof(ligne), f) != NULL && nombreMembres < MAX_MEMBRES) {
        Membre m;
        if (sscanf(ligne, "%d;%99[^\n]", &m.id, m.nom) == 2) {
            membres[nombreMembres] = m;
            nombreMembres++;
        }
    }
    fclose(f);
}

/* LOGIQUE : cree un membre. Si idCree n'est pas NULL, y ecrit l'id du nouveau membre. */
Resultat creerMembre(const char *nom, int *idCree) {
    if (strlen(nom) == 0) {
        return RES_NOM_VIDE;
    }
    if (strchr(nom, ';') != NULL) {
        return RES_CARACTERE_INTERDIT;
    }
    if (nombreMembres >= MAX_MEMBRES) {
        return RES_LIMITE_MEMBRES;
    }

    Membre nouveau;
    nouveau.id = prochainIdMembre();
    strncpy(nouveau.nom, nom, TAILLE_NOM - 1);
    nouveau.nom[TAILLE_NOM - 1] = '\0';

    membres[nombreMembres] = nouveau;
    nombreMembres++;
    sauvegarderMembres();

    if (idCree != NULL) {
        *idCree = nouveau.id;
    }
    return RES_OK;
}

/* CONSOLE */
void ajouterMembre(void) {
    char nom[TAILLE_NOM];
    printf("Nom du membre : ");
    lireTexte(nom, TAILLE_NOM);

    int id;
    Resultat r = creerMembre(nom, &id);
    if (r == RES_OK) {
        printf("Membre ajoute avec l'id %d.\n", id);
    } else {
        printf("%s\n", messageResultat(r));
    }
}
void afficherMembres(void) {
    if (nombreMembres == 0) {
        printf("Aucun membre enregistre.\n");
        return;
    }
    printf("\n%-4s %s\n", "ID", "Nom");
    printf("------------------------------\n");
    for (int i = 0; i < nombreMembres; i++) {
        printf("%-4d %s\n", membres[i].id, membres[i].nom);
    }
}
/*fonction supprimer membre*/
/* LOGIQUE */
Resultat retirerMembre(int id) {
    /* On cherche la position du membre dans le tableau */
    int pos = -1;
    for (int i = 0; i < nombreMembres; i++) {
        if (membres[i].id == id) {
            pos = i;
            break;
        }
    }

    if (pos == -1) {
        return RES_MEMBRE_INTROUVABLE;
    }
    if (nombreEmpruntsMembre(id) > 0) {
        return RES_MEMBRE_A_EMPRUNTS;
    }

    /* On decale les membres suivants d'une case vers la gauche */
    for (int i = pos; i < nombreMembres - 1; i++) {
        membres[i] = membres[i + 1];
    }
    nombreMembres--;
    sauvegarderMembres();
    return RES_OK;
}

/* CONSOLE */
void supprimerMembre(void) {
    printf("Id du membre a supprimer : ");
    int id = lireEntier();

    Resultat r = retirerMembre(id);
    if (r == RES_OK) {
        printf("Membre supprime.\n");
    } else {
        printf("%s\n", messageResultat(r));
    }
}
/*fonction membre existe */

int membreExiste(int id) {
    for (int i = 0; i < nombreMembres; i++) {
        if (membres[i].id == id) {
            return 1;
        }
    }
    return 0;
}

const char *nomMembre(int id) {
    for (int i = 0; i < nombreMembres; i++) {
        if (membres[i].id == id) {
            return membres[i].nom;
        }
    }
    return "Inconnu";
}



int nombreDeMembres(void) {
    return nombreMembres;
}

/* Renvoie l'adresse du membre a cette position (0, 1, 2...), ou NULL si elle n'existe pas */
const Membre *membreNumero(int position) {
    if (position < 0 || position >= nombreMembres) {
        return NULL;
    }
    return &membres[position];
}
