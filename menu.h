#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <locale.h>


#define VERSION "1.4"

#define mvaddstrc(pos_y, txt)	mvaddstr(pos_y, (COLS - strlen(txt)) / 2, txt)


//Structure des entrees:
struct entree
{
	char nom[200];
	char cmd[500];
	bool delai_necessaire;
	bool demander_apres;
	bool necessite_fork;
	bool maniere_de_quitter;
	bool cachee;
};

struct entree* entrees = NULL;


//Aide:
char aide [3500] = "echo \"Aide du Menu Terminal\n---------------------\n\n1. UTILISATION\nCe programme remplace la ligne de commande (shell) de base de l'OS, en ce sens où il permet d'ouvrir tous les programmes qui sont entrés dans son fichier de configuration.\nCe programme a l'avantage de présenter ces applications sous forme de menu TUI.\nAinsi, vous pouvez utiliser les flèches vers le haut et le bas pour choisir l'application que vous voulez exécuter, puis entrer Enter pour l'ouvrir.\nUne ligne de commande de base est aussi incluse dans ce programme.\n\n2. CONTRÔLES\nVoici la liste des contrôles du menu:\nFlèches vers le haut et le bas -> monte et descend dans le menu\nEnter ou Espace -> Démarre le programme sélectionné\nA -> Affiche ce texte\nR -> redessine l'écran du programme\nC ou / -> Ouvre la ligne de commande intégrée du programme\nQ -> Ferme le programme\n\nVoici la liste des contrôles de la ligne de commande intégrée:\nEnter -> Effectuer la commande\nFlèche vers le haut -> Copier la commande précédente\nFlèche vers le bas -> Effacer la commande actuelle\nEscape -> Fermer la ligne de commande et revenir au menu interactif\nAppuyez sur Backspace pour effacer et sur n'importe quelle lettre pour écrire. Les accents ne sont pas supportés.\n\n3. ARGUMENTS AU DÉMARRAGE\nCe programme accepte plusieurs arguments à son démarrage.\nCes options permettent de personnaliser le comportement de l'application et tiennent lieu de réglages.\nSi vous voulez les configurer de façon permanente, vous pouvez vous créer un alias approprié (par exemple <<alias menu=\\\"menu -e 0\\\">> (sans les << >>)).\nEntrez <<menu -?>> (sans les << >>) pour en savoir plus.\n\n4. FICHIER DE CONFIGURATION\nSi ces arguments peuvent servir à personnaliser l'interface ou le comportement général du programme, la liste des programmes inscrits dans le menu, elle, est configurable via le fichier de configuration.\nCe fichier, nommé \\\"config\\\", est situé dans le répertoire ~/.config/menu/ (dans votre votre home). (Il s'agit donc du fichier ~/.config/menu/config)\nCe fichier suit une syntaxe particulière, qui y est expliquée.\n\nPour faire bref, chaque entrée du menu doit être sur sa propre ligne et [ encadrée comme ceci ].\nIl est très important de laisser un espace entre n'importe quel caractère et les crochets.\nTout texte qui n'est pas encadré de cette facon est ignoré par le programme.\nÀ l'intérieur de ces crochets doivent se trouver 3 \\\"champs\\\" separés par des ; eux-mêmes entourés par des espaces (comme les crochets).\nLe premier champ est le nom de l'entrée, soit le texte qui sera affiché dans le menu.\nLe deuxième champ est la commande à exécuter pour cette entrée. Il peut s'agir d'une commande normale ou d'un script (si c'est un script, assurez-vous de fournir son chemin d'accès complet!). Notez bien que les alias et les fonctions définies dans ~/.bashrc ne sont pas accessible par le programme.\nLe troisième champ est optionnel. Il peut contenir un nombre illimité de flags. La liste de ces flags ainsi que leur explication est donnée dans le fichier de configuration.\nIl est conseillé de modifier ce fichier avec GNU nano, qui y appliquera automatiquement une coloration syntaxique si vous avez bien formées vos entrées (ce n'est toutefois pas une garantie qu'il n'y a pas d'erreurs de syntaxe).\n\n6. À PROPOS\nCe programme a été codé par Nicolas Audette en C avec ncurses.\n\"";


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

//Parametres modifiables:
bool fbterm = FALSE;
bool afficher_ligne_aide = TRUE;
int espacement = 1;
int delai = 2;
char nom_fconfig[100] = "";


//Fonctions:
void erreur(char[], char[], int);
void gestion_arguments(char[]);
bool lecture_fconfig();
int main(int argc, char* argv[]);
void quitter(char[]);
void rafraichir();
int taille_nbre(int);