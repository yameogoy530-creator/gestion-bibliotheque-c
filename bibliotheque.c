#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "membre.h"
#include "bibliotheque.h"
#include "utils.h"


#define DUREE_EMPRUNT_JOURS 14  /* duree maximale d'un emprunt */
#define MAX_EMPRUNTS_PAR_MEMBRE 3  /* nombre max de livres empruntes en meme temps */

/* "static" = ces variables sont privées à ce fichier :
 * seules les fonctions de bibliotheque.c peuvent les utiliser */
static Livre *bibliotheque = NULL;  /* pointeur vers le tableau, alloué à la demande */
static int nombreLivres = 0;        /* combien de livres sont enregistrés */
static int capacite = 0;            /* combien de livres le tableau peut contenir */

/* ---------- Fonctions internes (non visibles depuis main.c) ---------- */

/* Cherche un livre par son id. Retourne sa position dans le tableau, ou -1 */
static int trouverParId(int id) {
    for (int i = 0; i < nombreLivres; i++) {
        if (bibliotheque[i].id == id) {
            return i;
        }
    }
    return -1;
}

/* Retourne un id qui n'est utilise par aucun livre */
static int prochainId(void) {
    int max = 0;
    for (int i = 0; i < nombreLivres; i++) {
        if (bibliotheque[i].id > max) {
            max = bibliotheque[i].id;
        }
    }
    return max + 1;
}


/* S'assure qu'il reste une place libre pour un nouveau livre.
 * Retourne 1 si c'est bon, 0 s'il n'y a plus assez de memoire. */
static int assurerCapacite(void) {
    if (nombreLivres < capacite) {
        return 1;  /* il reste de la place */
    }

    int nouvelleCapacite = (capacite == 0) ? 10 : capacite * 2;
    Livre *agrandi = realloc(bibliotheque, nouvelleCapacite * sizeof(Livre));
    if (agrandi == NULL) {
        return 0;  /* echec : l'ancien tableau reste intact */
    }

    bibliotheque = agrandi;
    capacite = nouvelleCapacite;
    return 1;
}


/* Sauvegarde les livres dans un fichier texte */
static void sauvegarderLivres(void) {
    FILE *f = fopen("livres.txt", "w");
    if (f == NULL) {
        printf("Erreur : impossible de sauvegarder.\n");
        return;
    }

    for (int i = 0; i < nombreLivres; i++) {
            fprintf(f, "%d;%s;%s;%d;%d;%ld\n",
                bibliotheque[i].id,
                bibliotheque[i].titre,
                bibliotheque[i].auteur,
                bibliotheque[i].disponible,
                bibliotheque[i].idMembre,
                bibliotheque[i].dateEmprunt);
    }

    fclose(f);
}


/* ---------- Fonctions publiques (déclarées dans bibliotheque.h) ---------- */

/* Charge les livres a l'ouverture du programme */
void chargerLivres(void) {
    FILE *f = fopen("livres.txt", "r");
    if (f == NULL) {
        return;  /* pas de fichier = premier lancement, rien à charger */
    }

    char ligne[300];
    while (fgets(ligne, sizeof(ligne), f) != NULL && assurerCapacite()) {
        Livre l;
        l.idMembre = 0;
        l.dateEmprunt = 0;

        int lus = sscanf(ligne, "%d;%99[^;];%99[^;];%d;%d;%ld",
                         &l.id, l.titre, l.auteur, &l.disponible,
                         &l.idMembre, &l.dateEmprunt);

        /* 4, 5 ou 6 champs : tous les anciens formats restent acceptes */
        if (lus >= 4) {
            bibliotheque[nombreLivres] = l;
            nombreLivres++;
        }
    }

    fclose(f);
}


/* LOGIQUE : cree un livre. Si idCree n'est pas NULL, y ecrit l'id du nouveau livre. */
Resultat creerLivre(const char *titre, const char *auteur, int *idCree) {
    if (strlen(titre) == 0 || strlen(auteur) == 0) {
        return RES_CHAMP_VIDE;
    }
    /* Le ';' casserait le fichier de sauvegarde */
    if (strchr(titre, ';') != NULL || strchr(auteur, ';') != NULL) {
        return RES_CARACTERE_INTERDIT;
    }
    if (!assurerCapacite()) {
        return RES_MEMOIRE;
    }

    Livre nouveau;
    nouveau.id = prochainId();
    strncpy(nouveau.titre, titre, TAILLE_TEXTE - 1);
    nouveau.titre[TAILLE_TEXTE - 1] = '\0';
    strncpy(nouveau.auteur, auteur, TAILLE_TEXTE - 1);
    nouveau.auteur[TAILLE_TEXTE - 1] = '\0';
    nouveau.disponible = 1;
    nouveau.idMembre = 0;
    nouveau.dateEmprunt = 0;

    bibliotheque[nombreLivres] = nouveau;
    nombreLivres++;
    sauvegarderLivres();

    if (idCree != NULL) {
        *idCree = nouveau.id;
    }
    return RES_OK;
}

/* CONSOLE : lit au clavier, appelle la logique, affiche le resultat */
void ajouterLivre(void) {
    char titre[TAILLE_TEXTE];
    char auteur[TAILLE_TEXTE];

    printf("Titre  : ");
    lireTexte(titre, TAILLE_TEXTE);
    printf("Auteur : ");
    lireTexte(auteur, TAILLE_TEXTE);

    int id;
    Resultat r = creerLivre(titre, auteur, &id);
    if (r == RES_OK) {
        printf("Livre ajoute avec l'id %d.\n", id);
    } else {
        printf("%s\n", messageResultat(r));
    }
}
/*afficherLivres*/
void afficherLivres(void) {
    if (nombreLivres == 0) {
        printf("Aucun livre enregistre.\n");
        return;
    }

    printf("\n%-4s %-28s %-18s %-11s %s\n", "ID", "Titre", "Auteur", "Statut", "Emprunteur");
    printf("------------------------------------------------------------------------------\n");
    for (int i = 0; i < nombreLivres; i++) {
        printf("%-4d %-28s %-18s %-11s %s\n",
               bibliotheque[i].id,
               bibliotheque[i].titre,
               bibliotheque[i].auteur,
               bibliotheque[i].disponible ? "Disponible" : "Emprunte",
               bibliotheque[i].disponible ? "-" : nomMembre(bibliotheque[i].idMembre));
    }
}
/* nouvelle */
/* Affiche un livre sur une ligne (utilisee par la recherche) */
static void afficherUnLivre(const Livre *l) {
    printf("[%d] %s - %s (%s",
           l->id,
           l->titre,
           l->auteur,
           l->disponible ? "Disponible" : "Emprunte");

    if (!l->disponible) {
        printf(" par %s", nomMembre(l->idMembre));
    }
    printf(")\n");
}
/* LOGIQUE : dit si un livre correspond a un critere (1 = oui, 0 = non).
 * "texte" ne sert que pour CRIT_TITRE et CRIT_AUTEUR. */
int livreCorrespond(const Livre *l, int critere, const char *texte) {
    char champ[TAILLE_TEXTE];
    char recherche[TAILLE_TEXTE];

    switch (critere) {
        case CRIT_TITRE:
        case CRIT_AUTEUR:
            /* On compare en minuscules, sur des copies */
            strncpy(champ, (critere == CRIT_TITRE) ? l->titre : l->auteur, TAILLE_TEXTE - 1);
            champ[TAILLE_TEXTE - 1] = '\0';
            strncpy(recherche, texte, TAILLE_TEXTE - 1);
            recherche[TAILLE_TEXTE - 1] = '\0';
            enMinuscules(champ);
            enMinuscules(recherche);
            return strstr(champ, recherche) != NULL;

        case CRIT_DISPONIBLE:
            return l->disponible;

        case CRIT_EMPRUNTE:
            return !l->disponible;

        case CRIT_RETARD:
            return joursDeRetard(l) >= 0;
    }
    return 0;
}
/*recherche*/

/* CONSOLE */
void rechercherLivre(void) {
    printf("\nRechercher par :\n");
    printf("1. Titre\n");
    printf("2. Auteur\n");
    printf("3. Livres disponibles\n");
    printf("4. Livres empruntes\n");
    printf("5. Livres en retard\n");
    printf("Votre choix : ");
    int critere = lireEntier();

    if (critere < 1 || critere > 5) {
        printf("Choix invalide.\n");
        return;
    }

    /* Pour le titre et l'auteur, on demande le texte a chercher */
    char recherche[TAILLE_TEXTE] = "";
    if (critere == CRIT_TITRE || critere == CRIT_AUTEUR) {
        printf("Texte a rechercher : ");
        lireTexte(recherche, TAILLE_TEXTE);
    }

    int trouve = 0;
    for (int i = 0; i < nombreLivres; i++) {
        if (livreCorrespond(&bibliotheque[i], critere, recherche)) {
            afficherUnLivre(&bibliotheque[i]);
            trouve = 1;
        }
    }

    if (!trouve) {
        printf("Aucun livre trouve.\n");
    }
}
/*fonction comptage */
/* Compte combien de livres un membre a actuellement empruntes */
int nombreEmpruntsMembre(int idMembre) {
    int total = 0;
    for (int i = 0; i < nombreLivres; i++) {
        if (!bibliotheque[i].disponible && bibliotheque[i].idMembre == idMembre) {
            total++;
        }
    }
    return total;
}
/* emprunter*/

/* LOGIQUE */
Resultat faireEmprunter(int idLivre, int idMembre) {
    int pos = trouverParId(idLivre);
    if (pos == -1) {
        return RES_LIVRE_INTROUVABLE;
    }
    if (!bibliotheque[pos].disponible) {
        return RES_DEJA_EMPRUNTE;
    }
    if (!membreExiste(idMembre)) {
        return RES_MEMBRE_INTROUVABLE;
    }
    if (nombreEmpruntsMembre(idMembre) >= MAX_EMPRUNTS_PAR_MEMBRE) {
        return RES_LIMITE_ATTEINTE;
    }

    bibliotheque[pos].disponible = 0;
    bibliotheque[pos].idMembre = idMembre;
    bibliotheque[pos].dateEmprunt = (long)time(NULL);
    sauvegarderLivres();
    return RES_OK;
}

/* CONSOLE */
void emprunterLivre(void) {
    printf("Id du livre a emprunter : ");
    int idLivre = lireEntier();
    printf("Id du membre qui emprunte : ");
    int idMembre = lireEntier();

    Resultat r = faireEmprunter(idLivre, idMembre);
    if (r == RES_OK) {
        int pos = trouverParId(idLivre);
        printf("%s a emprunte : %s\n", nomMembre(idMembre), bibliotheque[pos].titre);
    } else {
        printf("%s\n", messageResultat(r));
    }
}

/* LOGIQUE */
Resultat faireRendre(int idLivre) {
    int pos = trouverParId(idLivre);
    if (pos == -1) {
        return RES_LIVRE_INTROUVABLE;
    }
    if (bibliotheque[pos].disponible) {
        return RES_NON_EMPRUNTE;
    }

    bibliotheque[pos].disponible = 1;
    bibliotheque[pos].idMembre = 0;
    bibliotheque[pos].dateEmprunt = 0;
    sauvegarderLivres();
    return RES_OK;
}

/* CONSOLE */
void rendreLivre(void) {
    printf("Id du livre a rendre : ");
    int id = lireEntier();

    Resultat r = faireRendre(id);
    if (r == RES_OK) {
        printf("Livre rendu. Merci !\n");
    } else {
        printf("%s\n", messageResultat(r));
    }
}
/*supprimerLivre*/
/* LOGIQUE */
Resultat retirerLivre(int idLivre) {
    int pos = trouverParId(idLivre);
    if (pos == -1) {
        return RES_LIVRE_INTROUVABLE;
    }
    if (!bibliotheque[pos].disponible) {
        return RES_DEJA_EMPRUNTE;  /* on ne supprime pas un livre emprunte */
    }

    /* On decale les livres suivants d'une case vers la gauche */
    for (int i = pos; i < nombreLivres - 1; i++) {
        bibliotheque[i] = bibliotheque[i + 1];
    }
    nombreLivres--;
    sauvegarderLivres();
    return RES_OK;
}

/* CONSOLE */
void supprimerLivre(void) {
    printf("Id du livre a supprimer : ");
    int id = lireEntier();

    Resultat r = retirerLivre(id);
    if (r == RES_OK) {
        printf("Livre supprime.\n");
    } else {
        printf("%s\n", messageResultat(r));
    }
}
/* Combien de livres y a-t-il ? */
int nombreDeLivres(void) {
    return nombreLivres;
}

/* Renvoie l'adresse du livre a cette position (0, 1, 2...), ou NULL si elle n'existe pas.
 * "const" = l'appelant peut lire le livre mais pas le modifier. */
const Livre *livreNumero(int position) {
    if (position < 0 || position >= nombreLivres) {
        return NULL;
    }
    return &bibliotheque[position];
}
/*alineas*/

/* Convertit un nombre de secondes en date lisible, ex : 25/12/2026 */
static void formaterDate(long secondes, char *resultat, int taille) {
    time_t t = (time_t)secondes;
    struct tm *info = localtime(&t);
    if (info == NULL) {
        strcpy(resultat, "?");
        return;
    }
    strftime(resultat, taille, "%d/%m/%Y", info);
}
/* LOGIQUE : renvoie le nombre de jours de retard du livre, ou -1 s'il n'est pas en retard
 * (livre disponible, date inconnue, ou delai pas encore depasse) */
int joursDeRetard(const Livre *l) {
    if (l->disponible || l->dateEmprunt == 0) {
        return -1;
    }

    long secondesParJour = 24L * 3600L;
    long limite = DUREE_EMPRUNT_JOURS * secondesParJour;
    long ecoule = (long)time(NULL) - l->dateEmprunt;

    if (ecoule <= limite) {
        return -1;
    }
    return (int)((ecoule - limite) / secondesParJour);
}

/* CONSOLE */
void afficherRetards(void) {
    int trouve = 0;

    for (int i = 0; i < nombreLivres; i++) {
        int jours = joursDeRetard(&bibliotheque[i]);
        if (jours >= 0) {
            char date[20];
            formaterDate(bibliotheque[i].dateEmprunt, date, sizeof(date));

            printf("[%d] %s - emprunte le %s par %s : %d jour(s) de retard\n",
                   bibliotheque[i].id,
                   bibliotheque[i].titre,
                   date,
                   nomMembre(bibliotheque[i].idMembre),
                   jours);
            trouve = 1;
        }
    }

    if (!trouve) {
        printf("Aucun retard.\n");
    }
}
/* Traduit un code de resultat en message lisible */
/* Traduit un code de resultat en message lisible */
const char *messageResultat(Resultat r) {
    switch (r) {
        case RES_OK:                 return "Operation reussie.";
        case RES_MEMOIRE:            return "Erreur : plus assez de memoire.";
        case RES_CHAMP_VIDE:         return "Le titre et l'auteur ne peuvent pas etre vides.";
        case RES_CARACTERE_INTERDIT: return "Le caractere ';' est interdit.";
        case RES_LIVRE_INTROUVABLE:  return "Livre introuvable.";
        case RES_DEJA_EMPRUNTE:      return "Ce livre est actuellement emprunte.";
        case RES_NON_EMPRUNTE:       return "Ce livre n'etait pas emprunte.";
        case RES_MEMBRE_INTROUVABLE: return "Membre introuvable.";
        case RES_LIMITE_ATTEINTE:    return "Ce membre a atteint la limite d'emprunts.";
        case RES_NOM_VIDE:           return "Le nom ne peut pas etre vide.";
        case RES_MEMBRE_A_EMPRUNTS:  return "Impossible : ce membre a encore des livres empruntes.";
        case RES_LIMITE_MEMBRES:     return "Nombre maximum de membres atteint.";
    }
    return "Erreur inconnue.";
}
/*alineas2*/
void libererLivres(void) {
    free(bibliotheque);
    bibliotheque = NULL;
    nombreLivres = 0;
    capacite = 0;
}
/*fonction comparaison */
/* ---------- Tri ---------- */

/* Compare deux textes sans tenir compte des majuscules */
static int comparerTextes(const char *a, const char *b) {
    char ma[TAILLE_TEXTE];
    char mb[TAILLE_TEXTE];
    strcpy(ma, a);
    strcpy(mb, b);
    enMinuscules(ma);
    enMinuscules(mb);
    return strcmp(ma, mb);
}

/* qsort donne des pointeurs "void *" : on les convertit en "Livre *" */
static int comparerParTitre(const void *a, const void *b) {
    const Livre *la = (const Livre *)a;
    const Livre *lb = (const Livre *)b;
    return comparerTextes(la->titre, lb->titre);
}

static int comparerParAuteur(const void *a, const void *b) {
    const Livre *la = (const Livre *)a;
    const Livre *lb = (const Livre *)b;
    return comparerTextes(la->auteur, lb->auteur);
}

static int comparerParId(const void *a, const void *b) {
    const Livre *la = (const Livre *)a;
    const Livre *lb = (const Livre *)b;
    return la->id - lb->id;
}
/*fonction comparaison*/
/* LOGIQUE : trie le tableau sur place selon un critere (TRI_TITRE, TRI_AUTEUR ou TRI_ID),
 * puis sauvegarde le nouvel ordre */
void trierLivresPar(int critere) {
    if (nombreLivres < 2) {
        return;  /* rien a trier */
    }

    if (critere == TRI_TITRE) {
        qsort(bibliotheque, nombreLivres, sizeof(Livre), comparerParTitre);
    } else if (critere == TRI_AUTEUR) {
        qsort(bibliotheque, nombreLivres, sizeof(Livre), comparerParAuteur);
    } else if (critere == TRI_ID) {
        qsort(bibliotheque, nombreLivres, sizeof(Livre), comparerParId);
    } else {
        return;  /* critere inconnu */
    }

    sauvegarderLivres();
}

/* CONSOLE */
void trierLivres(void) {
    if (nombreLivres < 2) {
        printf("Pas assez de livres a trier.\n");
        return;
    }

    printf("\nTrier par :\n");
    printf("1. Titre\n");
    printf("2. Auteur\n");
    printf("3. Id\n");
    printf("Votre choix : ");
    int critere = lireEntier();

    if (critere < 1 || critere > 3) {
        printf("Choix invalide.\n");
        return;
    }

    trierLivresPar(critere);
    afficherLivres();
}
