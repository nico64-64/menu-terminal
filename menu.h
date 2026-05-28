#ifndef _MENU_H
#define _MENU_H

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
	bool cachee; //indique que cette entrée ne doit pas être affichée dans le menu car ses conditions d'affichage ne sont pas réunies (flag x11-*, twin-* ou fbterm-*)
};


//Fonctions:
void erreur(char[], char[], int);
void gestion_arguments(char[]);
bool lecture_fconfig();
int main(int argc, char* argv[]);
void quitter(char[]);
void rafraichir();
int taille_nbre(int);

#endif //_MENU_H
