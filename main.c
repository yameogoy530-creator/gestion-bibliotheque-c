#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bibliotheque.h"
#include "membre.h"

/* ---------- Identifiants des controles ---------- */
#define ID_LISTE                 100
#define ID_TITRE                 101
#define ID_AUTEUR                102
#define ID_BTN_AJOUTER           103
#define ID_COMBO_MEMBRE          104
#define ID_BTN_EMPRUNTER         105
#define ID_BTN_RENDRE            106
#define ID_BTN_SUPPRIMER         107
#define ID_LISTE_MEMBRES         108
#define ID_NOM_MEMBRE            109
#define ID_BTN_AJOUTER_MEMBRE    110
#define ID_BTN_SUPPRIMER_MEMBRE  111
#define ID_COMBO_CRITERE         112
#define ID_CHAMP_RECHERCHE       113
#define ID_BTN_RECHERCHER        114
#define ID_BTN_TOUT_AFFICHER     115

static HWND liste;           /* le tableau des livres */
static HWND champTitre;      /* la case de texte du titre */
static HWND champAuteur;     /* la case de texte de l'auteur */
static HWND comboMembre;     /* la liste deroulante des membres */
static HWND listeMembres;    /* le tableau des membres */
static HWND champNomMembre;  /* la case de texte du nom d'un nouveau membre */
static HWND comboCritere;    /* la liste deroulante des criteres de recherche */
static HWND champRecherche;  /* la case de texte de la recherche */
static HFONT policeApp;      /* la police systeme de Windows (Segoe UI) */

/* Filtre actuellement applique au tableau des livres (0 = aucun filtre) */
static int filtreCritere = 0;
static char filtreTexte[TAILLE_TEXTE] = "";

/* Annonces : ces fonctions sont definies plus bas */
static void remplirListe(void);
static void remplirListeMembres(void);
static void remplirComboMembres(void);


/* ---------- Tableau des livres ---------- */

/* Cree les colonnes du tableau des livres */
static void creerColonnes(void) {
    LVCOLUMN col;
    memset(&col, 0, sizeof(col));
    col.mask = LVCF_TEXT | LVCF_WIDTH;

    col.pszText = "ID";         col.cx = 50;
    ListView_InsertColumn(liste, 0, &col);
    col.pszText = "Titre";      col.cx = 200;
    ListView_InsertColumn(liste, 1, &col);
    col.pszText = "Auteur";     col.cx = 140;
    ListView_InsertColumn(liste, 2, &col);
    col.pszText = "Statut";     col.cx = 90;
    ListView_InsertColumn(liste, 3, &col);
    col.pszText = "Emprunteur"; col.cx = 110;
    ListView_InsertColumn(liste, 4, &col);
    col.pszText = "Retard";     col.cx = 60;
    ListView_InsertColumn(liste, 5, &col);
}

/* Efface le tableau puis le remplit avec les livres qui passent le filtre */
static void remplirListe(void) {
    ListView_DeleteAllItems(liste);

    int rang = 0;  /* numero de la prochaine ligne du tableau */

    for (int i = 0; i < nombreDeLivres(); i++) {
        const Livre *l = livreNumero(i);

        /* Si un filtre est actif, on saute les livres qui ne correspondent pas */
        if (filtreCritere != 0 && !livreCorrespond(l, filtreCritere, filtreTexte)) {
            continue;
        }

        char texte[20];
        sprintf(texte, "%d", l->id);

        /* Premiere colonne : on cree la ligne et on cache l'id du livre derriere */
        LVITEM ligne;
        memset(&ligne, 0, sizeof(ligne));
        ligne.mask = LVIF_TEXT | LVIF_PARAM;
        ligne.iItem = rang;
        ligne.iSubItem = 0;
        ligne.pszText = texte;
        ligne.lParam = l->id;
        ListView_InsertItem(liste, &ligne);

        /* Autres colonnes : on remplit les cases de cette ligne */
        ListView_SetItemText(liste, rang, 1, (LPSTR)l->titre);
        ListView_SetItemText(liste, rang, 2, (LPSTR)l->auteur);
        ListView_SetItemText(liste, rang, 3, (LPSTR)(l->disponible ? "Disponible" : "Emprunte"));
        ListView_SetItemText(liste, rang, 4, (LPSTR)(l->disponible ? "-" : nomMembre(l->idMembre)));

        int jours = joursDeRetard(l);
        if (jours >= 0) {
            sprintf(texte, "%d j", jours);
            ListView_SetItemText(liste, rang, 5, texte);
        } else {
            ListView_SetItemText(liste, rang, 5, (LPSTR)"-");
        }

        rang++;
    }
}
/* Cree la police que Windows utilise pour ses propres fenetres (Segoe UI sur Windows 10 et 11) */
static void creerPolice(void) {
    NONCLIENTMETRICS ncm;
    memset(&ncm, 0, sizeof(ncm));
    ncm.cbSize = sizeof(ncm);

    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0)) {
        policeApp = CreateFontIndirect(&ncm.lfMessageFont);
    } else {
        policeApp = (HFONT)GetStockObject(DEFAULT_GUI_FONT);  /* solution de secours */
    }
}

/* ---------- Creation des controles ---------- */
static HWND creerControle(HWND parent, HINSTANCE hInstance, const char *classe, const char *texte,
                          DWORD style, int x, int y, int largeur, int hauteur, int id) {
    /* Les etiquettes (STATIC) n'ont pas besoin de la touche Tab, les autres controles oui */
    DWORD tab = (strcmp(classe, "STATIC") == 0) ? 0 : WS_TABSTOP;

    HWND controle = CreateWindow(classe, texte, WS_CHILD | WS_VISIBLE | tab | style,
                                 x, y, largeur, hauteur,
                                 parent, (HMENU)(INT_PTR)id, hInstance, NULL);
        SendMessage(controle, WM_SETFONT, (WPARAM)policeApp, TRUE);
    return controle;
}

/* ---------- Actions sur les livres ---------- */

/* Appelee quand on clique sur "Ajouter" (livre) */
static void actionAjouter(HWND fenetre) {
    char titre[TAILLE_TEXTE];
    char auteur[TAILLE_TEXTE];

    /* On lit le texte tape dans les deux cases */
    GetWindowText(champTitre, titre, sizeof(titre));
    GetWindowText(champAuteur, auteur, sizeof(auteur));

    /* Meme fonction "logique" que la version console */
    Resultat r = creerLivre(titre, auteur, NULL);
    if (r != RES_OK) {
        MessageBox(fenetre, messageResultat(r), "Erreur", MB_OK | MB_ICONWARNING);
        return;
    }

    /* Succes : on vide les cases, on rafraichit le tableau */
    SetWindowText(champTitre, "");
    SetWindowText(champAuteur, "");
    remplirListe();
    SetFocus(champTitre);
}

/* Renvoie l'id du livre selectionne dans le tableau, ou -1 si aucune ligne n'est selectionnee.
 * L'id est cache derriere chaque ligne (voir remplirListe) : cela marche meme quand le tableau
 * est filtre, car le numero de la ligne ne correspond plus a la position du livre. */
static int idLivreSelectionne(HWND fenetre) {
    int ligne = ListView_GetNextItem(liste, -1, LVNI_SELECTED);
    if (ligne == -1) {
        MessageBox(fenetre, "Selectionnez d'abord un livre dans le tableau.",
                   "Information", MB_OK | MB_ICONINFORMATION);
        return -1;
    }

    LVITEM item;
    memset(&item, 0, sizeof(item));
    item.mask = LVIF_PARAM;
    item.iItem = ligne;
    ListView_GetItem(liste, &item);
    return (int)item.lParam;
}

/* Si l'operation a reussi : on rafraichit les tableaux. Sinon : on affiche l'erreur. */
static void traiterResultat(HWND fenetre, Resultat r) {
    if (r == RES_OK) {
        remplirListe();
        remplirListeMembres();  /* le nombre d'emprunts d'un membre a peut-etre change */
    } else {
        MessageBox(fenetre, messageResultat(r), "Erreur", MB_OK | MB_ICONWARNING);
    }
}

static void actionEmprunter(HWND fenetre) {
    int idLivre = idLivreSelectionne(fenetre);
    if (idLivre == -1) {
        return;
    }

    /* Quelle ligne de la liste deroulante est choisie ? */
    int choix = (int)SendMessage(comboMembre, CB_GETCURSEL, 0, 0);
    if (choix == CB_ERR) {
        MessageBox(fenetre, "Choisissez d'abord un membre dans la liste deroulante.",
                   "Information", MB_OK | MB_ICONINFORMATION);
        return;
    }

    /* On recupere l'id du membre, cache derriere cette ligne */
    int idMembre = (int)SendMessage(comboMembre, CB_GETITEMDATA, choix, 0);

    traiterResultat(fenetre, faireEmprunter(idLivre, idMembre));
}

static void actionRendre(HWND fenetre) {
    int idLivre = idLivreSelectionne(fenetre);
    if (idLivre == -1) {
        return;
    }
    traiterResultat(fenetre, faireRendre(idLivre));
}

static void actionSupprimer(HWND fenetre) {
    int idLivre = idLivreSelectionne(fenetre);
    if (idLivre == -1) {
        return;
    }

    /* On demande confirmation avant de supprimer */
    if (MessageBox(fenetre, "Supprimer definitivement ce livre ?", "Confirmation",
                   MB_YESNO | MB_ICONQUESTION) != IDYES) {
        return;
    }
    traiterResultat(fenetre, retirerLivre(idLivre));
}

/* Appelee quand on clique sur "Rechercher" */
static void actionRechercher(HWND fenetre) {
    int choix = (int)SendMessage(comboCritere, CB_GETCURSEL, 0, 0);
    if (choix == CB_ERR) {
        MessageBox(fenetre, "Choisissez d'abord un critere de recherche.",
                   "Information", MB_OK | MB_ICONINFORMATION);
        return;
    }

    /* L'ordre de la liste deroulante est celui des constantes CRIT_TITRE ... CRIT_RETARD */
    filtreCritere = choix + 1;
    GetWindowText(champRecherche, filtreTexte, sizeof(filtreTexte));
    remplirListe();
}

/* Appelee quand on clique sur "Tout afficher" */
static void actionToutAfficher(void) {
    filtreCritere = 0;
    filtreTexte[0] = '\0';
    SetWindowText(champRecherche, "");
    remplirListe();
}


/* ---------- Membres ---------- */

static void creerColonnesMembres(void) {
    LVCOLUMN col;
    memset(&col, 0, sizeof(col));
    col.mask = LVCF_TEXT | LVCF_WIDTH;

    col.pszText = "ID";       col.cx = 50;
    ListView_InsertColumn(listeMembres, 0, &col);
    col.pszText = "Nom";      col.cx = 200;
    ListView_InsertColumn(listeMembres, 1, &col);
    col.pszText = "Emprunts"; col.cx = 80;
    ListView_InsertColumn(listeMembres, 2, &col);
}

/* Efface le tableau des membres puis le remplit */
static void remplirListeMembres(void) {
    ListView_DeleteAllItems(listeMembres);

    for (int i = 0; i < nombreDeMembres(); i++) {
        const Membre *m = membreNumero(i);
        char texte[20];

        sprintf(texte, "%d", m->id);
        LVITEM ligne;
        memset(&ligne, 0, sizeof(ligne));
        ligne.mask = LVIF_TEXT;
        ligne.iItem = i;
        ligne.iSubItem = 0;
        ligne.pszText = texte;
        ListView_InsertItem(listeMembres, &ligne);

        ListView_SetItemText(listeMembres, i, 1, (LPSTR)m->nom);

        sprintf(texte, "%d", nombreEmpruntsMembre(m->id));
        ListView_SetItemText(listeMembres, i, 2, texte);
    }
}

/* Remplit la liste deroulante, en gardant le membre deja choisi s'il existe encore */
static void remplirComboMembres(void) {
    int idChoisi = -1;
    int choix = (int)SendMessage(comboMembre, CB_GETCURSEL, 0, 0);
    if (choix != CB_ERR) {
        idChoisi = (int)SendMessage(comboMembre, CB_GETITEMDATA, choix, 0);
    }

    SendMessage(comboMembre, CB_RESETCONTENT, 0, 0);

    for (int i = 0; i < nombreDeMembres(); i++) {
        const Membre *m = membreNumero(i);
        char texte[TAILLE_NOM + 20];
        sprintf(texte, "%d - %s", m->id, m->nom);

        int index = (int)SendMessage(comboMembre, CB_ADDSTRING, 0, (LPARAM)texte);
        SendMessage(comboMembre, CB_SETITEMDATA, index, (LPARAM)m->id);

        if (m->id == idChoisi) {
            SendMessage(comboMembre, CB_SETCURSEL, index, 0);
        }
    }
}

static void actionAjouterMembre(HWND fenetre) {
    char nom[TAILLE_NOM];
    GetWindowText(champNomMembre, nom, sizeof(nom));

    Resultat r = creerMembre(nom, NULL);
    if (r != RES_OK) {
        MessageBox(fenetre, messageResultat(r), "Erreur", MB_OK | MB_ICONWARNING);
        return;
    }

    SetWindowText(champNomMembre, "");
    remplirListeMembres();
    remplirComboMembres();
    SetFocus(champNomMembre);
}

static void actionSupprimerMembre(HWND fenetre) {
    int ligne = ListView_GetNextItem(listeMembres, -1, LVNI_SELECTED);
    if (ligne == -1) {
        MessageBox(fenetre, "Selectionnez d'abord un membre dans le tableau de droite.",
                   "Information", MB_OK | MB_ICONINFORMATION);
        return;
    }

    const Membre *m = membreNumero(ligne);
    if (m == NULL) {
        return;
    }

    if (MessageBox(fenetre, "Supprimer definitivement ce membre ?", "Confirmation",
                   MB_YESNO | MB_ICONQUESTION) != IDYES) {
        return;
    }

    /* Apres retirerMembre, le tableau interne est decale : "m" ne doit plus etre utilise */
    Resultat r = retirerMembre(m->id);
    if (r != RES_OK) {
        MessageBox(fenetre, messageResultat(r), "Erreur", MB_OK | MB_ICONWARNING);
        return;
    }

    remplirListeMembres();
    remplirComboMembres();
}


/* ---------- La fenetre ---------- */

/* Appelee par Windows a chaque evenement de la fenetre */
LRESULT CALLBACK ProcedureFenetre(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {

        case WM_CREATE: {
            HINSTANCE hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
                        creerPolice();

            /* --- Rangee 1 : ajouter un livre --- */
            creerControle(hwnd, hInstance, "STATIC", "Titre :", 0, 10, 14, 50, 20, 0);
            champTitre = creerControle(hwnd, hInstance, "EDIT", "",
                                       WS_BORDER | ES_AUTOHSCROLL, 65, 10, 230, 24, ID_TITRE);

            creerControle(hwnd, hInstance, "STATIC", "Auteur :", 0, 310, 14, 55, 20, 0);
            champAuteur = creerControle(hwnd, hInstance, "EDIT", "",
                                        WS_BORDER | ES_AUTOHSCROLL, 370, 10, 200, 24, ID_AUTEUR);

            creerControle(hwnd, hInstance, "BUTTON", "Ajouter",
                          BS_PUSHBUTTON, 585, 9, 100, 26, ID_BTN_AJOUTER);

            /* --- Rangee 2 : emprunter / rendre / supprimer un livre --- */
            creerControle(hwnd, hInstance, "STATIC", "Membre :", 0, 10, 54, 60, 20, 0);
            comboMembre = creerControle(hwnd, hInstance, "COMBOBOX", "",
                                        CBS_DROPDOWNLIST | WS_VSCROLL, 75, 50, 210, 200, ID_COMBO_MEMBRE);

            creerControle(hwnd, hInstance, "BUTTON", "Emprunter",
                          BS_PUSHBUTTON, 295, 49, 95, 26, ID_BTN_EMPRUNTER);
            creerControle(hwnd, hInstance, "BUTTON", "Rendre",
                          BS_PUSHBUTTON, 400, 49, 95, 26, ID_BTN_RENDRE);
            creerControle(hwnd, hInstance, "BUTTON", "Supprimer",
                          BS_PUSHBUTTON, 505, 49, 95, 26, ID_BTN_SUPPRIMER);

            /* --- Rangee 3 : rechercher --- */
            creerControle(hwnd, hInstance, "STATIC", "Chercher :", 0, 10, 94, 65, 20, 0);
            comboCritere = creerControle(hwnd, hInstance, "COMBOBOX", "",
                                         CBS_DROPDOWNLIST | WS_VSCROLL, 80, 90, 130, 200, ID_COMBO_CRITERE);
            /* Meme ordre que les constantes CRIT_TITRE, CRIT_AUTEUR, CRIT_DISPONIBLE, CRIT_EMPRUNTE, CRIT_RETARD */
            SendMessage(comboCritere, CB_ADDSTRING, 0, (LPARAM)"Titre");
            SendMessage(comboCritere, CB_ADDSTRING, 0, (LPARAM)"Auteur");
            SendMessage(comboCritere, CB_ADDSTRING, 0, (LPARAM)"Disponibles");
            SendMessage(comboCritere, CB_ADDSTRING, 0, (LPARAM)"Empruntes");
            SendMessage(comboCritere, CB_ADDSTRING, 0, (LPARAM)"En retard");
            SendMessage(comboCritere, CB_SETCURSEL, 0, 0);

            champRecherche = creerControle(hwnd, hInstance, "EDIT", "",
                                           WS_BORDER | ES_AUTOHSCROLL, 220, 90, 190, 24, ID_CHAMP_RECHERCHE);
            creerControle(hwnd, hInstance, "BUTTON", "Rechercher",
                          BS_PUSHBUTTON, 420, 89, 100, 26, ID_BTN_RECHERCHER);
            creerControle(hwnd, hInstance, "BUTTON", "Tout afficher",
                          BS_PUSHBUTTON, 530, 89, 110, 26, ID_BTN_TOUT_AFFICHER);

            /* --- Tableau des livres (a gauche) --- */
            liste = CreateWindow(WC_LISTVIEW, "",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                                 10, 130, 700, 280,
                                 hwnd, (HMENU)ID_LISTE, hInstance, NULL);
                       ListView_SetExtendedListViewStyle(liste, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
            SendMessage(liste, WM_SETFONT, (WPARAM)policeApp, TRUE);

            /* --- Panneau des membres (a droite) --- */
            creerControle(hwnd, hInstance, "STATIC", "Nom du membre :", 0, 725, 14, 105, 20, 0);
            champNomMembre = creerControle(hwnd, hInstance, "EDIT", "",
                                           WS_BORDER | ES_AUTOHSCROLL, 835, 10, 170, 24, ID_NOM_MEMBRE);
            creerControle(hwnd, hInstance, "BUTTON", "Ajouter",
                          BS_PUSHBUTTON, 1015, 9, 100, 26, ID_BTN_AJOUTER_MEMBRE);
            creerControle(hwnd, hInstance, "BUTTON", "Supprimer le membre",
                          BS_PUSHBUTTON, 725, 49, 160, 26, ID_BTN_SUPPRIMER_MEMBRE);

            listeMembres = CreateWindow(WC_LISTVIEW, "",
                                        WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                                        725, 130, 390, 280,
                                        hwnd, (HMENU)ID_LISTE_MEMBRES, hInstance, NULL);
                        ListView_SetExtendedListViewStyle(listeMembres, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
            SendMessage(listeMembres, WM_SETFONT, (WPARAM)policeApp, TRUE);

            /* --- On prepare les colonnes puis on remplit tout --- */
            creerColonnes();
            creerColonnesMembres();
            remplirListe();
            remplirListeMembres();
            remplirComboMembres();
            return 0;
        }

        case WM_COMMAND:  /* un clic sur un bouton */
            switch (LOWORD(wParam)) {
                case ID_BTN_AJOUTER:           actionAjouter(hwnd);          break;
                case ID_BTN_EMPRUNTER:         actionEmprunter(hwnd);        break;
                case ID_BTN_RENDRE:            actionRendre(hwnd);           break;
                case ID_BTN_SUPPRIMER:         actionSupprimer(hwnd);        break;
                case ID_BTN_AJOUTER_MEMBRE:    actionAjouterMembre(hwnd);    break;
                case ID_BTN_SUPPRIMER_MEMBRE:  actionSupprimerMembre(hwnd);  break;
                case ID_BTN_RECHERCHER:        actionRechercher(hwnd);       break;
                case ID_BTN_TOUT_AFFICHER:     actionToutAfficher();         break;
            }
            return 0;

        case WM_NOTIFY: {  /* un tableau nous previent d'un evenement (ici : clic sur un en-tete) */
            NMHDR *info = (NMHDR *)lParam;

            if (info->hwndFrom == liste && info->code == LVN_COLUMNCLICK) {
                NMLISTVIEW *clic = (NMLISTVIEW *)lParam;

                /* Colonnes : 0 = ID, 1 = Titre, 2 = Auteur */
                int critere = 0;
                if (clic->iSubItem == 0) {
                    critere = TRI_ID;
                } else if (clic->iSubItem == 1) {
                    critere = TRI_TITRE;
                } else if (clic->iSubItem == 2) {
                    critere = TRI_AUTEUR;
                }

                if (critere != 0) {
                    trierLivresPar(critere);
                    remplirListe();
                }
            }
            return 0;
        }

        case WM_SIZE:  /* la fenetre change de taille : les tableaux suivent */
            if (liste != NULL && listeMembres != NULL) {
                int largeurMembres = LOWORD(lParam) - 735;
                if (largeurMembres < 100) {
                    largeurMembres = 100;
                }
                MoveWindow(liste, 10, 130, 700, HIWORD(lParam) - 140, TRUE);
                MoveWindow(listeMembres, 725, 130, largeurMembres, HIWORD(lParam) - 140, TRUE);
            }
            return 0;
                    case WM_SETFOCUS:  /* la fenetre reprend le focus : on le donne a la case "Titre" */
            if (champTitre != NULL) {
                SetFocus(champTitre);
            }
            return 0;

        case WM_DESTROY:
            DeleteObject(policeApp);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR ligneCommande, int modeAffichage) {
    InitCommonControls();  /* necessaire pour utiliser les tableaux */

    /* On charge les donnees, exactement comme dans la version console */
    chargerMembres();
    chargerLivres();

    WNDCLASS classe = {0};
    classe.lpfnWndProc = ProcedureFenetre;
    classe.hInstance = hInstance;
    classe.lpszClassName = "FenetreBibliotheque";
    classe.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    classe.hCursor = LoadCursor(NULL, IDC_ARROW);
    classe.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(101));
    RegisterClass(&classe);

    HWND fenetre = CreateWindow("FenetreBibliotheque", "Gestion de bibliotheque",
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 1150, 560,
                                NULL, NULL, hInstance, NULL);
    ShowWindow(fenetre, modeAffichage);

        MSG message;
    while (GetMessage(&message, NULL, 0, 0)) {
        /* Touche Entree : on declenche le bouton qui correspond a la case ou se trouve le curseur */
        if (message.message == WM_KEYDOWN && message.wParam == VK_RETURN) {
            HWND focus = GetFocus();
            int idBouton = 0;

            if (focus == champTitre || focus == champAuteur) {
                idBouton = ID_BTN_AJOUTER;
            } else if (focus == champRecherche) {
                idBouton = ID_BTN_RECHERCHER;
            } else if (focus == champNomMembre) {
                idBouton = ID_BTN_AJOUTER_MEMBRE;
            }

            if (idBouton != 0) {
                SendMessage(fenetre, WM_COMMAND, (WPARAM)idBouton, 0);
                continue;  /* touche traitee : on passe au message suivant */
            }
        }

        /* Touche Tab : IsDialogMessage deplace le curseur d'un controle a l'autre */
        if (IsDialogMessage(fenetre, &message)) {
            continue;
        }

        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    libererLivres();
    return 0;
}
