#include "menu.h"


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
		execl("/usr/bin/bash", "bash", "-c", buffer, NULL);
		erreur("Impossible d'executer cette commande lors de la fermeture du programme.", cmd, 0); //ne devrait jamais arriver
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

		printf("Ce petit programme genere un menu ncurses permettant de lancer un programme de son choix.\n");
		printf("Il est possible de le faire appeler en lieu et place de la ligne de commande Bash.\n\n");

		printf("Voici la liste des arguments que peut recevoir ce programme:\n");
		printf("--aide (-?) -> Affiche ce texte et quitte.\n");
		printf("--version (-v) -> Affiche la version du programme et quitte.\n");
		printf("--fbterm -> Force l'ouverture du programme en mode fbterm (affiche un fond sombre derrière les entrées du menu) (se fait automatiquement si le programme roule sur fbterm).\n");
		printf("--normal -> Force l'ouverture du programme en mode normal (se fait automatiquement si le programme ne roule pas sur fbterm).\n");
		printf("--sansaide (-s) -> N'affiche pas la ligne d'aide en bas de l'ecran.\n");
		printf("--espacement (-e) -> Indique que le prochain argument sera le nombre de lignes vides a laisser entre chaque entree (de 0 a 6 seulement).\n");
		printf("--delai (-d) -> Indique que le prochain argument sera le delai (en secondes, de 0 a 6 seulement) a laisser apres l'execution d'une programme qui l'exige.\n");
		printf("--fconfig (-f) -> Indique que le prochain argument sera le chemin d'acces au fichier de configuration (sans espace!).\n");

		exit(0);
	}

	else if (!strcmp(arg, "--fbterm"))
	{fbterm = TRUE;}

	else if (!strcmp(arg, "--normal"))
	{fbterm = FALSE;}

	else if (!strcmp(arg, "--sansaide") || !strcmp(arg, "-s"))
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
	{erreur("Le fichier de configuration ne contient aucune entree valide!", (char*) nom_fconfig, 0); return FALSE;}

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
			{erreur("Le fichier de configuration contient une entree sans nom! (valeur = numero de l'entree)", (char*) nom_fconfig, c); return FALSE;}
			strcpy(entrees[c].nom, mot);
			mot = strtok(NULL, " \t\n");
			while (mot != NULL && strcmp(mot, ";") != 0 && strcmp(mot, ",") != 0 && strcmp(mot, ":") != 0)
			{strcat(entrees[c].nom, " "); strcat(entrees[c].nom, mot); mot = strtok(NULL, " \t\n");}
			if (!c || strlen(entrees[c].nom) + 4 > longueur_sel)
			{longueur_sel = strlen(entrees[c].nom) + 4;}

			//cmd:
			mot = strtok(NULL, " \t\n");
			if (mot == NULL || !strcmp(mot, ";") || !strcmp(mot, ",") || !strcmp(mot, ":"))
			{erreur("Le fichier de configuration contient une entree sans nom! (valeur = numero de l'entree)", (char*) nom_fconfig, c); return FALSE;}
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
				else if (!strcmp(mot, "twin-on"))
				{
					if (getenv("TWDISPLAY") == NULL) //getenv renvoie la valeur de la variable bash specifiee en parametre ou NULL si elle n'existe pas
					{entrees[c].cachee = TRUE; nbre_cachees++;}
				}
				else if (!strcmp(mot, "twin-off"))
				{
					if (getenv("TWDISPLAY") != NULL)
					{entrees[c].cachee = TRUE; nbre_cachees++;}
				}
				else if (!strcmp(mot, "fbterm-off"))
				{
					if (getenv("FBTERM") != NULL)
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
	pthread_t thread2;


	//Mise en mode UTF-8 (si ce n'est pas daja le cas):
	setlocale(LC_ALL, "en_CA.UTF-8");

	//Vérification de la présence de notre hint fbterm:
	if (getenv("FBTERM") != NULL)
	{fbterm = TRUE;}

	//Gestion des arguments:
	for (unsigned c = 1; c < argc; c++)
	{gestion_arguments(argv[c]);}

	//Trouve le fichier de configuration de l'utilisateur s'il n'a pas ete specifie:
	if (!strcmp(nom_fconfig, ""))
	{
		strcpy(nom_fconfig, getenv("HOME"));
		strcat(nom_fconfig, "/.config/menu/config");
	}

	//Lecture des entrees depuis le fichier de configuration:
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

	//Pipe l'aide a less pour qu'on puisse facilement la consulter:
	strcat(aide, " | less");

	//S'assure que le curseur n'est pas sur un 1er choix cache:
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


	//Main Loop:
	while (1)
	{
		do
		{input = getch();} while (input == ERR);

		//Ligne de commande:
		if (cmd_line)
		{
			//Ajout d'une lettre a la commande:
			if (isprint(input))
			{
				commande[strlen(commande) + 1] = '\000';
				commande[strlen(commande)] = input;
			}

			//Suppression de la derniere lettre de la commande:
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

			//Commande precedente:
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
		//Rafraichir:
		case KEY_RESIZE:
		case 'R':
		case 'r':
			clear(); //pour etre sur de bien nettoyer le terminal...
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

		//Choisir une entree:
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
					{erreur("Impossible de creer un nouveau thread pour executer cette commande!", "Avez-vous abuse des commandes forkees?", choix); getch();}
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
			endwin();
			system("clear");
			system(aide);
			break;

		//Quitter:
		case 'Q':
		case 'q':
			quitter("");
			break;
		}

		rafraichir();
	}
}