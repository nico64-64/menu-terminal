#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <locale.h>
#include "menu.h"

#define VERSION "1.6"


//Liste des entrées dans le menu:
struct entree* entrees = NULL;

//Aide:
char aide[] = "echo \"Aide du Menu Terminal\n---------------------\n\n1. UTILISATION\nCe programme remplace la ligne de commande (shell) de base de l'OS, en ce sens où il permet d'ouvrir tous les programmes qui sont entrés dans son fichier de configuration.\nCe programme a l'avantage de présenter ces applications sous forme de menu TUI.\nAinsi, vous pouvez utiliser les flèches vers le haut et le bas pour choisir l'application que vous voulez exécuter, puis entrer Enter pour l'ouvrir.\nUne ligne de commande de base est aussi incluse dans ce programme.\n\n2. CONTRÔLES\nVoici la liste des contrôles du menu:\nFlèches vers le haut et le bas -> monte et descend dans le menu\nEnter ou Espace -> Démarre le programme sélectionné\nA -> Affiche ce texte\nR -> redessine l'écran du programme\nC -> Ouvre la ligne de commande intégrée du programme\n/ -> (Même chose)\nQ -> Ferme le programme\n\nVoici la liste des contrôles de la ligne de commande intégrée:\nReturn / Enter      -> Effectuer la commande\nFlèche vers le haut -> Copier la commande précédente\nFlèche vers le bas  -> Effacer la commande actuelle\nEscape              -> Fermer la ligne de commande et revenir au menu\n* Les accents ne sont pas supportés. *\n\n3. ARGUMENTS AU DÉMARRAGE\nCe programme accepte plusieurs arguments à son démarrage.\nCes options permettent de personnaliser le comportement du programme et tiennent lieu de réglages.\nSi vous voulez les configurer de façon permanente, vous pouvez vous créer un alias approprié (par exemple en ajoutant alias menu=\\\"menu -e 0\\\" à votre ~/.bashrc).\nEntrez menu -? pour en savoir plus.\n\n4. FICHIER DE CONFIGURATION\nSi ces arguments peuvent servir à personnaliser l'interface ou le comportement général du programme, la liste des programmes inscrits dans le menu, elle, est gérée par le fichier de configuration du programme, c'est-à-dire le fichier ~/.config/menu/config.\nCe fichier suit une syntaxe particulière, qui y est expliquée.\n\nPour faire bref, chaque entrée du menu doit être sur sa propre ligne et [ encadrée comme ceci ].\nIl est très important de laisser un espace entre n'importe quel caractère et les crochets.\nTout texte qui n'est pas encadré de cette facon est ignoré par le programme.\nÀ l'intérieur de ces crochets doivent se trouver 3 \\\"champs\\\" separés par des ; eux-mêmes entourés par des espaces (comme les crochets).\nLe premier champ est le nom de l'entrée, soit le texte qui sera affiché dans le menu.\nLe deuxième champ est la commande à exécuter pour cette entrée. Il peut s'agir d'une commande normale ou d'un script (si c'est un script, assurez-vous de fournir son chemin d'accès complet). Notez bien que les alias et les fonctions définies dans ~/.bashrc ne sont pas accessible par le programme.\nLe troisième champ est optionnel. Il peut contenir un nombre illimité de flags. La liste de ces flags ainsi que leur explication est donnée dans le fichier de configuration.\nIl est conseillé de modifier ce fichier avec GNU nano, qui y appliquera automatiquement une coloration syntaxique si vous avez bien formées vos entrées (ce n'est toutefois pas une garantie qu'il n'y a pas d'erreurs de syntaxe).\n\n6. À PROPOS\nCe programme a été codé par Nicolas Audette en C avec ncurses.\n\" | less";

//Variables globales:
int nbre_entrees = 0;
int longueur_sel;
char nom_tty[20] = "ERREUR!";
int pos_debut_liste;
int choix = 0;
bool ncurses_active = FALSE;
int nbre_cachees = 0; //nombre d'entrees cachees (parce que leurs conditions ne sont pas remplies)
bool cmd_line = FALSE;
char commande[100] = "";
char historique[100] = "";

//Parametres modifiables via les options d'invocation:
bool fbterm = FALSE;
bool afficher_ligne_aide = TRUE;
bool souris = TRUE;
int espacement = 1;
int delai = 2;
char nom_fconfig[100] = ""; //valeur par défaut (déterminée au démarrage): $HOME/.config/menu/config


int taille_nbre (int nbre)
//Trouve le nombre de caracteres occupes par le nombre recu en parametre.
{
	unsigned compteur = 0;
	int n = nbre;

	if (n < 0)
	{compteur++; n *= -1;}
	for (; n >= 10; compteur++)
	{n /= 10;}

	return compteur;
}


void quitter (char cmd[])
//Ferme le programme en liberant les ressources nesessaires et en reinitialisant le terminal.
{
	char buffer[300] = "\"";

	endwin();
	ncurses_active = FALSE;
	if (entrees != NULL)
	{free(entrees);}

	if (cmd == "")
	{exit(0);}
	else if (cmd != NULL)
	{
		strcat(buffer, cmd);
		strcat(buffer, "\"");
		execl("/usr/bin/sh", "sh", "-c", buffer, NULL);
		erreur("Impossible d'exécuter cette commande lors de la fermeture du programme.", cmd, 0); //ne devrait pas arriver
		exit(1000);
	}
}


void erreur (char msg[], char details[], int valeur)
//Affiche un message d'erreur.
{
	if (ncurses_active)
	{
		rafraichir();
		mvprintw(LINES - 5, (COLS - strlen(msg) - strlen("Erreur: ")) / 2, "Erreur: %s", msg);
		mvprintw(LINES - 4, (COLS - strlen(details) - strlen("Details: ")) / 2, "Details: %s", details);
		mvprintw(LINES - 3, (COLS - taille_nbre(valeur) - strlen("Valeur: ")) / 2, "Valeur: %d", valeur);
	}
	else
	{printf("\nErreur: %s\nDetails: %s\nValeur: %d\n", msg, details, valeur);}
}


void gestion_arguments (char arg[])
//Gere les arguments recus par le programme.
{
	static bool fconfig_en_attente = FALSE;
	static bool espacement_en_attente = FALSE;
	static bool delai_en_attente = FALSE;
	int* ptr_int = NULL;


	if (espacement_en_attente || delai_en_attente)
	{
		if (delai_en_attente)
		{ptr_int = &delai; delai_en_attente = FALSE;}
		else
		{ptr_int = &espacement; espacement_en_attente = FALSE;}

		if (!strcmp(arg, "0"))
		{*ptr_int = 0;}
		else if (!strcmp(arg, "1"))
		{*ptr_int = 1;}
		else if (!strcmp(arg, "2"))
		{*ptr_int = 2;}
		else if (!strcmp(arg, "3"))
		{*ptr_int = 3;}
		else if (!strcmp(arg, "4"))
		{*ptr_int = 4;}
		else if (!strcmp(arg, "5"))
		{*ptr_int = 5;}
		else if (!strcmp(arg, "6"))
		{*ptr_int = 6;}
		else
		{printf("L'espacement ou le delai fourni en argument n'est pas valide.\n"); sleep(2);}
	}

	else if (fconfig_en_attente)
	{strcpy(nom_fconfig, arg); fconfig_en_attente = FALSE;}

	else if (!strcmp(arg, "--version") || !strcmp(arg, "-v"))
	{printf("version %s\n", VERSION); exit(0);}

	else if (!strcmp(arg, "--help") || !strcmp(arg, "-h") || !strcmp(arg, "--aide") || !strcmp(arg, "-a") || !strcmp(arg, "-?"))
	{
		printf("Menu Terminal\n\n");
		printf("Usage:\nmenu [--arguments]\n\n");

		printf("Ce petit programme génère un menu TUI permettant de lancer un programme de son choix.\n");
		printf("Il est possible de le faire appeler en lieu et place de la ligne de commande.\n\n");

		printf("Voici la liste des arguments que peut recevoir ce programme:\n");
		printf("--aide (-?)        -> Affiche ce texte et quitte.\n");
		printf("--delai (-d)       -> Indique que le prochain argument sera le délai (en secondes, de 0 à 6 seulement) a laisser après l'exécution d'une programme qui l'exige.\n");
		printf("--espacement (-e)  -> Indique que le prochain argument sera le nombre de lignes vides a laisser entre chaque entrée (de 0 à 6 seulement).\n");
		printf("--fbterm           -> Force l'ouverture du programme en mode fbterm (affiche un fond sombre derrière les entrées du menu) (se fait automatiquement si le programme roule sur fbterm).\n");
		printf("--fconfig (-f)     -> Indique que le prochain argument sera le chemin d'accès au fichier de configuration (sans espace!).\n");
		printf("--normal           -> Force l'ouverture du programme en mode normal (se fait automatiquement si le programme ne roule pas sur fbterm).\n");
		printf("--sans-aide (-s)   -> N'affiche pas la ligne d'aide en bas de l'écran.\n");
		printf("--sans-souris (-S) -> Désactive la prise en charge de la souris.\n");
		printf("--version (-v)     -> Affiche la version du programme et quitte.\n");

		exit(0);
	}

	else if (!strcmp(arg, "--fbterm"))
	{fbterm = TRUE;}

	else if (!strcmp(arg, "--normal"))
	{fbterm = FALSE;}
	
	else if (!strcmp(arg, "--sans-souris") || !strcmp(arg, "-S"))
	{souris = FALSE;}

	else if (!strcmp(arg, "--sans-aide") || !strcmp(arg, "-s"))
	{afficher_ligne_aide = FALSE;}

	else if (!strcmp(arg, "--fconfig") || !strcmp(arg, "--config") || !strcmp(arg, "-f"))
	{fconfig_en_attente = TRUE;}

	else if (!strcmp(arg, "--espacement") || !strcmp(arg, "-e"))
	{espacement_en_attente = TRUE;}

	else if (!strcmp(arg, "--delai") || !strcmp(arg, "-d"))
	{delai_en_attente = TRUE;}

	else
	{printf("\nArgument Invalide!\nL'argument suivant n'est pas un argument valide: \"%s\"\nEntrez \"menu -?\" pour en savoir plus.\n", arg); exit(0);}
}


bool lecture_fconfig ()
//Lit et applique le fichier de configuration du programme.
//Renvoie TRUE en cas de reussite et FALSE en cas d'echec.
{
	FILE* fconfig = fopen(nom_fconfig, "r");
	char ligne[300];
	char* mot;

	if (fconfig == NULL)
	{erreur("Impossible d'ouvrir le fichier de configuration!", (char*) nom_fconfig, 0); return FALSE;}

	//Trouve le nombre d'entrees inscrites dans le fichier:
	nbre_entrees = 0;
	while (fgets(ligne, sizeof(ligne), fconfig) != NULL)
	{
		mot = strtok(ligne, " \t\n");
		if (mot != NULL && !strcmp(mot, "["))
		{nbre_entrees++;}
	}!
	fclose(fconfig);

	if (!nbre_entrees)
	{erreur("Le fichier de configuration ne contient aucune entrée valide!", (char*) nom_fconfig, 0); return FALSE;}

	//Initialise l'array des entrees:
	entrees = calloc(nbre_entrees, sizeof(struct entree));

	//Lit les entrees du fichier:
	fconfig = fopen(nom_fconfig, "r");
	for (unsigned c = 0; c < nbre_entrees; )
	{
		//Initialisation des flags a leurs valeurs par defaut:
		entrees[c].demander_apres = FALSE;
		entrees[c].delai_necessaire = FALSE;
		entrees[c].necessite_fork = FALSE;
		entrees[c].maniere_de_quitter = FALSE;
		entrees[c].cachee = FALSE;

		fgets(ligne, sizeof(ligne), fconfig);
		mot = strtok(ligne, " \t\n");
		if (mot != NULL && !strcmp(mot, "["))
		{
			//nom:
			mot = strtok(NULL, " \t\n");
			if (mot == NULL || !strcmp(mot, ";") || !strcmp(mot, ",") || !strcmp(mot, ":") || !strcmp(mot, ":") || !strcmp(mot, "]"))
			{erreur("Le fichier de configuration contient une entrée sans nom! (valeur = numéro de l'entrée)", (char*) nom_fconfig, c); return FALSE;}
			strcpy(entrees[c].nom, mot);
			mot = strtok(NULL, " \t\n");
			while (mot != NULL && strcmp(mot, ";") != 0 && strcmp(mot, ",") != 0 && strcmp(mot, ":") != 0)
			{strcat(entrees[c].nom, " "); strcat(entrees[c].nom, mot); mot = strtok(NULL, " \t\n");}
			if (!c || strlen(entrees[c].nom) + 4 > longueur_sel)
			{longueur_sel = strlen(entrees[c].nom) + 4;}

			//cmd:
			mot = strtok(NULL, " \t\n");
			if (mot == NULL || !strcmp(mot, ";") || !strcmp(mot, ",") || !strcmp(mot, ":"))
			{erreur("Le fichier de configuration contient une entrée sans nom! (valeur = numéro de l'entrée)", (char*) nom_fconfig, c); return FALSE;}
			strcpy(entrees[c].cmd, mot);
			mot = strtok(NULL, " \t\n");
			while (mot != NULL && strcmp(mot, ";") != 0 && strcmp(mot, ",") != 0 && strcmp(mot, ":") != 0)
			{strcat(entrees[c].cmd, " "); strcat(entrees[c].cmd, mot); mot = strtok(NULL, " \t\n");}

			//flags:
			mot = strtok(NULL, " \t\n");
			while (mot != NULL && strcmp(mot, "]") != 0)
			{
				if (!strcmp(mot, "delai"))
				{entrees[c].delai_necessaire = TRUE;}
				else if (!strcmp(mot, "demander"))
				{entrees[c].demander_apres = TRUE;}
				else if (!strcmp(mot, "fork"))
				{entrees[c].necessite_fork = TRUE;}
				else if (!strcmp(mot, "quitter"))
				{entrees[c].maniere_de_quitter = TRUE;}
				else if (!strcmp(mot, "x11-on"))
				{
					if (getenv("DISPLAY") == NULL) //getenv renvoie la valeur de la variable environnementale specifiée en parametre ou NULL si elle n'existe pas
					{entrees[c].cachee = TRUE; nbre_cachees++;}
				}
				else if (!strcmp(mot, "x11-off"))
				{
					if (getenv("DISPLAY") != NULL)
					{entrees[c].cachee = TRUE; nbre_cachees++;}
				}
				else if (!strcmp(mot, "twin-on"))
				{
					if (getenv("TWDISPLAY") == NULL)
					{entrees[c].cachee = TRUE; nbre_cachees++;}
				}
				else if (!strcmp(mot, "twin-off"))
				{
					if (getenv("TWDISPLAY") != NULL)
					{entrees[c].cachee = TRUE; nbre_cachees++;}
				}
				else if (!strcmp(mot, "fbterm-on"))
				{
					if (getenv("FBTERM_BACKGROUND_IMAGE") == NULL)
					{entrees[c].cachee = TRUE; nbre_cachees++;}
				}
				else if (!strcmp(mot, "fbterm-off"))
				{
					if (getenv("FBTERM_BACKGROUND_IMAGE") != NULL)
					{entrees[c].cachee = TRUE; nbre_cachees++;}
				}
				else
				{erreur("Le fichier de configuration contient une entree avec un flag inconnu! (valeur = numero de l'entree)", (char*) nom_fconfig, c); return FALSE;}

				mot = strtok(NULL, " \t\n");
			}
			if (mot == NULL)
			{erreur("Le fichier de configuration contient une entree incomplete! (valeur = numero de l'entree)", (char*) nom_fconfig, c); return FALSE;}

			c++;
		}
	}

	fclose(fconfig);
	return TRUE;
}


void rafraichir ()
//Affiche le menu a l'ecran.
{
	char buffer[120];

	erase();
	pos_debut_liste = (LINES - (nbre_entrees - nbre_cachees) * (espacement + 1)) / 2;

	//Contours:
	mvhline(0, 1, ACS_HLINE, COLS - 2);
	mvhline(LINES - 1, 1, ACS_HLINE, COLS - 2);
	mvvline(1, 0, ACS_VLINE, LINES - 2);
	mvvline(1, COLS - 1, ACS_VLINE, LINES - 2);
	mvaddch(0, 0, ACS_ULCORNER);
	mvaddch(0, COLS - 1, ACS_URCORNER);
	mvaddch(LINES - 1, 0, ACS_LLCORNER);
	mvaddch(LINES - 1, COLS - 1, ACS_LRCORNER);
	mvaddstrc(0, " Menu Terminal ");
	mvaddch(0, COLS - strlen(nom_tty) - 3, ' ');
	addstr(nom_tty);
	addch(' ');

	//Entrees:
	if (fbterm)
	{attrset(COLOR_PAIR(1) | A_BLINK);}
	for (unsigned c = 0, compte_cachees = 0; c < nbre_entrees; c++)
	{
		if (entrees[c].cachee)
		{compte_cachees++; continue;} //continue skippe le reste de cette iteration

		if (choix == c)
		{attrset(COLOR_PAIR(10)); mvhline(pos_debut_liste + (espacement + 1) * (c - compte_cachees), (COLS - longueur_sel) / 2, ' ', longueur_sel);}
		mvaddstrc(pos_debut_liste + (espacement + 1) * (c - compte_cachees), entrees[c].nom);
		if (choix == c && fbterm)
		{attrset(COLOR_PAIR(1) | A_BLINK);}
		else if (choix == c)
		{standend();}
	}
	standend();

	//Ligne de commande (si necessaire):
	if (cmd_line)
	{
		sprintf(buffer, "Cmd: [ %s ]", commande);
		mvaddstrc(LINES - 3, buffer);
	}

	//Ligne d'aide:
	else if (afficher_ligne_aide)
	{mvaddstrc(LINES - 3, "Entrez / pour une ligne de commande, A pour de l'aide ou Q pour quitter.");}

	//Affichage:
	refresh();
}


int main (int argc, char* argv[])
//S'occupe de l'initialisation du programme et de la main loop.
{
	int input;
	bool key_simulee = FALSE;
	MEVENT mev;
	pthread_t thread2;
	int nbre_entrees_cachees_avant;


	//Utilisation de la locale UTF-8 de base si on en pas déjà une:
	if (!setlocale(LC_ALL, ""))
	{setlocale(LC_ALL, "C.UTF-8");}

	//Vérification de la présence de notre hint fbterm:
	if (getenv("FBTERM_BACKGROUND_IMAGE") != NULL)
	{fbterm = TRUE;}

	//Gestion des arguments:
	for (unsigned c = 1; c < argc; c++)
	{gestion_arguments(argv[c]);}

	//Trouve le fichier de configuration de l'utilisateur s'il n'a pas été specifié:
	if (!strcmp(nom_fconfig, ""))
	{
		strcpy(nom_fconfig, getenv("HOME"));
		strcat(nom_fconfig, "/.config/menu/config");
	}

	//Lecture des entrées depuis le fichier de configuration:
	if (!lecture_fconfig())
	{quitter("");}

	//Trouve le nom du tty:
	strcpy(nom_tty, ttyname(STDIN_FILENO));
	for (unsigned c = 5; nom_tty[c-1] != '\000'; c++)
	{nom_tty[c-5] = nom_tty[c];}
	if (nom_tty[3] == '/')
	{
		for (unsigned c = 3; nom_tty[c] != '\000'; c++)
		{nom_tty[c] = nom_tty[c+1];}
	}

	//S'assure que l'entrée sélectionnée n'est pas cachée:
	while (entrees[choix].cachee)
	{choix++;}

	//Initialisation de ncurses et affichage:
	initscr();
	raw();
	nonl();
	keypad(stdscr, TRUE);
	noecho();
	ESCDELAY = 0;
	start_color();
	init_pair(10, COLOR_BLACK, COLOR_WHITE);
	init_pair(1, COLOR_WHITE, COLOR_BLACK); //pour fbterm
	curs_set(0);
	ncurses_active = TRUE;
	rafraichir();
	
	//Initialisation de la prise en charge de la souris (si activée):
	if (souris)
	{mousemask(ALL_MOUSE_EVENTS, NULL);}


	//Main Loop:
	while (1)
	{
		if (!key_simulee)
		{
			do
			{input = getch();} while (input == ERR);
		}
		else
		{key_simulee = FALSE;}

		//Ligne de commande:
		if (cmd_line)
		{
			//Ajout d'une lettre à la commande:
			if (isprint(input))
			{
				commande[strlen(commande) + 1] = '\000';
				commande[strlen(commande)] = input;
			}

			//Suppression de la dernière lettre de la commande:
			else if (input == KEY_BACKSPACE && strlen(commande) > 0)
			{commande[strlen(commande) - 1] = '\000';}

			//Envoi la commande:
			else if (input == 13) //enter
			{
				endwin();
				system("clear");

				system(commande);
				printf("\nAppuyez sur Enter pour revenir au menu Terminal.\n");
				getchar();

				cmd_line = FALSE;
				strcpy(historique, commande);
				strcpy(commande, "");
			}

			//Commande précédente:
			else if (input == KEY_UP)
			{strcpy(commande, historique);}

			//Effacer la commande:
			else if (input == KEY_DOWN)
			{strcpy(commande, "");}

			//Fermer la ligne de commande:
			else if (input == 27) //escape
			{cmd_line = FALSE; strcpy(commande, "");}

			rafraichir();
			continue;
		}

		//Menu regulier:
		switch (input)
		{
		//Redessiner:
		case KEY_RESIZE:
		case 'R':
		case 'r':
			clear(); //pour être sûr de bien nettoyer le terminal...
			break;

		//Descendre dans le menu:
		case KEY_DOWN:
			do
			{choix++;} while (choix < nbre_entrees && entrees[choix].cachee);
			while (choix >= nbre_entrees || entrees[choix].cachee)
			{choix--;}
			break;

		//Monter dans le menu:
		case KEY_UP:
			do
			{choix--;} while (choix > 0 && entrees[choix].cachee);
			while (choix < 0 || entrees[choix].cachee)
			{choix++;}
			break;

		//Choisir une entrée:
		case 13: //enter
		case ' ':
			if (entrees[choix].maniere_de_quitter)
			{quitter(entrees[choix].cmd);}
			else
			{
				endwin();
				system("clear");

				if (entrees[choix].necessite_fork)
				{
					if (pthread_create(&thread2, NULL, (void*) &system, &entrees[choix].cmd) != 0)
					{erreur("Impossible de creer un nouveau thread pour exécuter cette commande!", "Avez-vous abusé des commandes forkées?", choix); getch();}
				}
				else
				{system(entrees[choix].cmd);}

				if (entrees[choix].demander_apres)
				{printf("\nAppuyez sur Enter pour revenir au menu Terminal...\n"); getchar();}
				else if (entrees[choix].delai_necessaire)
				{sleep(delai);}
			}
			break;

		//Ligne de commande:
		case 'C':
		case 'c':
		case '/':
			cmd_line = TRUE;
			break;

		//Aide:
		case 'A':
		case 'a':
		case 'H':
		case 'h':
			endwin();
			system("clear");
			system(aide);
			break;

		//Quitter:
		case 'Q':
		case 'q':
			quitter("");
			break;
		
		//Souris:
		case KEY_MOUSE:
			getmouse(&mev);
			if (mev.bstate == 4 && mev.x >= (COLS - longueur_sel) / 2  && mev.x < (COLS + longueur_sel) / 2 /*clic gauche dans la zone centrale...*/ \
				&& mev.y >= pos_debut_liste && mev.y < pos_debut_liste + (nbre_entrees - nbre_cachees) * (espacement + 1) && (mev.y - pos_debut_liste) % (espacement + 1) == 0) //...sur une des entrées
			{
				choix = (mev.y - pos_debut_liste) / (espacement + 1);
				nbre_entrees_cachees_avant = 0;
				for (int i = 0; i <= choix + nbre_entrees_cachees_avant; i++)
				{
					if (entrees[i].cachee)
					{nbre_entrees_cachees_avant++;}
				}
				choix += nbre_entrees_cachees_avant;
				rafraichir();
				key_simulee = TRUE;
				input = 13;
			}
			else if (mev.bstate == 65536) //molette vers le haut (ne fonctionne pas sur tty)
			{key_simulee = TRUE; input = KEY_UP;}
			else if (mev.bstate == 2097152) //molette vers le bas (ne fonctionne pas sur tty)
			{key_simulee = TRUE; input = KEY_DOWN;}
			break;
		}

		rafraichir();
	}
}