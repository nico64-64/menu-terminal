# menu-terminal

Un petit menu facilement configurable pour démarrer des programmes depuis le terminal.

## Installation

Compilez le programme avec la commande suivante:
```bash
gcc menu.c -o menu -lncursesw
```

**Ce programme utilise ncurses, ce qui le rend impossible à porter sur Windows.**

Il devrait toutefois fonctionner sur n'importe quel système basé sur Unix (Linux, BSD, MacOS, etc.), en autant que ce système fournisse un bibliothèque ncurses.

## Configuration

Ce menu est facilement configurable via un fichier de configuration (`~/.config/menu/config`) qui doit être créé par l'utilisateur.

Un exemple d'un tel fichier est le fichier config.exemple fourni avec ce programme.<br>
Vous pouvez modifier, enlever et ajouter des entrées selon vos besoins, votre OS, les applications installées, etc.<br>
La syntaxe de ce fichier y est aussi expliquée.

## Usage

Utiliser ce menu est aussi simple que d'entrer `./menu` dans votre terminal (ou `menu` si vous l'avez copié dans votre `$PATH`).

Ce programme accepte quelques options de démarrage, qui sont affichées lorsque vous entrez `menu -h` (ou `menu -?`).

Un texte d'aide est disponible en entrant 'A' une fois le menu ouvert.

Vous pouvez entrer une commande spéciale en entrant 'C' ou '/' une fois le menu ouvert.

## Limitations

Ce programme ne vérifie pas si toutes les entrées tiennent dans le terminal, donc assurez-vous d'ajuster le nombre d'entrées (dans le fichier de configuration) ou l'espacement entre celles-ci (avec `menu -e 0` pour ne pas en avoir, par exemple) en conséquence.

Assurez-vous aussi d'avoir une syntaxe valide dans votre fichier de configuration et que vous appelez des commandes qui sont accessibles par le programme (évitez les alias, par exemple).

Ce programme ne supporte pas l'usage de la souris.

## Notes

Les options "twin-on" et "twin-off" du fichier de configuration peuvent être utilisées pour vérifier si ce menu est démarré depuis un terminal twin (voir https://github.com/cosmos72/twin).

L'option "fbterm-off" vérifiera si le terminal depuis lequel ce programme est démarré est une instance de fbterm (https://wiki.archlinux.org/title/Fbterm), mais cela nécessite que vous créiez un script ou une fonction qui settera la variable `$FBTERM` de votre shell en même temps que vous démarrez fbterm. Ce menu assume également que fbterm est utilisé pour afficher une image en arrière-plan, ce qui explique pourquoi si le programme détecte la variable `$FBTERM`, il affichera un fond gris derrière le texte des entrées du menu.
