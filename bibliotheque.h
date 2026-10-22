#ifndef BIBLIOTHEQUE_H
#define BIBLIOTHEQUE_H

#define MAX_LIVRES 100   /* nombre maximum de livres */
#define TAILLE_TEXTE 100 /* taille max d'un titre ou d'un auteur */
/* Criteres de recherche (ce sont aussi les numeros du menu console) */
#define CRIT_TITRE       1
#define CRIT_AUTEUR      2
#define CRIT_DISPONIBLE  3
#define CRIT_EMPRUNTE    4
#define CRIT_RETARD      5

/* Criteres de tri */
#define TRI_TITRE   1
#define TRI_AUTEUR  2
#define TRI_ID      3



/* Une "structure" regroupe plusieurs informations sur un même objet */
typedef struct {
    int id;
    char titre[TAILLE_TEXTE];
    char auteur[TAILLE_TEXTE];
    int disponible;  /* 1 = disponible, 0 = emprunté */
    int idMembre; /*ID DU MEMEBRE QUI LA EMPRUNTé    */
    long dateEmprunt;
} Livre;



int joursDeRetard(const Livre *l);   /* -1 si le livre n'est pas en retard */
int livreCorrespond(const Livre *l, int critere, const char *texte);
void trierLivresPar(int critere);
/*Structure resultat*/
/* Codes renvoyes par les fonctions "logique" */
typedef enum {
    RES_OK = 0,
    RES_MEMOIRE,
    RES_CHAMP_VIDE,
    RES_CARACTERE_INTERDIT,
    RES_LIVRE_INTROUVABLE,
    RES_DEJA_EMPRUNTE,
    RES_NON_EMPRUNTE,
    RES_MEMBRE_INTROUVABLE,
    RES_LIMITE_ATTEINTE,
    RES_NOM_VIDE,
    RES_MEMBRE_A_EMPRUNTS,
    RES_LIMITE_MEMBRES
} Resultat;

/* Fonctions que main.c a le droit d'appeler */
void chargerLivres(void);
void ajouterLivre(void);
void afficherLivres(void);
void rechercherLivre(void);
void emprunterLivre(void);
void rendreLivre(void);
void supprimerLivre(void);
void libererLivres(void);
void trierLivres(void);
void afficherRetards(void);
int nombreEmpruntsMembre(int idMembre);

/* Fonctions "logique" : ne lisent rien au clavier et n'affichent rien */
Resultat creerLivre(const char *titre, const char *auteur, int *idCree);
Resultat faireEmprunter(int idLivre, int idMembre);
Resultat faireRendre(int idLivre);
Resultat retirerLivre(int idLivre);
const char *messageResultat(Resultat r);
int nombreDeLivres(void);
const Livre *livreNumero(int position);

#endif
