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
	char nom[200]; //nom de l'entrée tel qu'affiché dans le menu
	char cmd[500]; //commande à exécuter lorsque l'entrée est sélectionnée
	bool delai_necessaire; //indique qu'il faut laisser un délai avant de revenir au menu (flag delai)
	bool demander_apres; //indique qu'il faut demander une confirmation avant de revenir au menu (flag demander)
	bool necessite_fork; //indique qu'il faut exécuter cette commande dans un autre thread (flag fork)
	bool maniere_de_quitter; //indique qu'il faut fermer le programme en exécutant cette commande (flag quitter)
	bool cachee; //indique que cette entrée ne doit pas être affichée dans le menu car ses conditions d'affichage ne sont pas réunies (flag x11-*, twin-* ou fbterm-off)
};

struct entree* entrees = NULL; //liste des entrées dans le menu


//Aide:
char aide [4000] = "echo \"Aide du Menu Terminal\n---------------------\n\n1. UTILISATION\nCe programme remplace la ligne de commande (shell) de base de l'OS, en ce sens où il permet d'ouvrir tous les programmes qui sont entrés dans son fichier de configuration.\nCe programme a l'avantage de présenter ces applications sous forme de menu TUI.\nAinsi, vous pouvez utiliser les flèches vers le haut et le bas pour choisir l'application que vous voulez exécuter, puis entrer Enter pour l'ouvrir.\nUne ligne de commande de base est aussi incluse dans ce programme.\n\n2. CONTRÔLES\nVoici la liste des contrôles du menu:\nFlèches vers le haut et le bas -> monte et descend dans le menu\nEnter ou Espace -> Démarre le programme sélectionné\nA -> Affiche ce texte\nR -> redessine l'écran du programme\nC -> Ouvre la ligne de commande intégrée du programme\n/ -> (Même chose)\nQ -> Ferme le programme\n\nVoici la liste des contrôles de la ligne de commande intégrée:\nReturn / Enter      -> Effectuer la commande\nFlèche vers le haut -> Copier la commande précédente\nFlèche vers le bas  -> Effacer la commande actuelle\nEscape              -> Fermer la ligne de commande et revenir au menu\n* Les accents ne sont pas supportés. *\n\n3. ARGUMENTS AU DÉMARRAGE\nCe programme accepte plusieurs arguments à son démarrage.\nCes options permettent de personnaliser le comportement du programme et tiennent lieu de réglages.\nSi vous voulez les configurer de façon permanente, vous pouvez vous créer un alias approprié (par exemple en ajoutant alias menu=\\\"menu -e 0\\\" à votre ~/.bashrc).\nEntrez menu -? pour en savoir plus.\n\n4. FICHIER DE CONFIGURATION\nSi ces arguments peuvent servir à personnaliser l'interface ou le comportement général du programme, la liste des programmes inscrits dans le menu, elle, est gérée par le fichier de configuration du programme, c'est-à-dire le fichier ~/.config/menu/config.\nCe fichier suit une syntaxe particulière, qui y est expliquée.\n\nPour faire bref, chaque entrée du menu doit être sur sa propre ligne et [ encadrée comme ceci ].\nIl est très important de laisser un espace entre n'importe quel caractère et les crochets.\nTout texte qui n'est pas encadré de cette facon est ignoré par le programme.\nÀ l'intérieur de ces crochets doivent se trouver 3 \\\"champs\\\" separés par des ; eux-mêmes entourés par des espaces (comme les crochets).\nLe premier champ est le nom de l'entrée, soit le texte qui sera affiché dans le menu.\nLe deuxième champ est la commande à exécuter pour cette entrée. Il peut s'agir d'une commande normale ou d'un script (si c'est un script, assurez-vous de fournir son chemin d'accès complet). Notez bien que les alias et les fonctions définies dans ~/.bashrc ne sont pas accessible par le programme.\nLe troisième champ est optionnel. Il peut contenir un nombre illimité de flags. La liste de ces flags ainsi que leur explication est donnée dans le fichier de configuration.\nIl est conseillé de modifier ce fichier avec GNU nano, qui y appliquera automatiquement une coloration syntaxique si vous avez bien formées vos entrées (ce n'est toutefois pas une garantie qu'il n'y a pas d'erreurs de syntaxe).\n\n6. À PROPOS\nCe programme a été codé par Nicolas Audette en C avec ncurses.\n\" | less";


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


//Fonctions:
void erreur(char[], char[], int);
void gestion_arguments(char[]);
bool lecture_fconfig();
int main(int argc, char* argv[]);
void quitter(char[]);
void rafraichir();
int taille_nbre(int);