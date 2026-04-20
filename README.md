# Menu Terminal

Un petit menu facilement configurable pour démarrer des programmes depuis le terminal.

## Installation
Vous devez d'abord installer les dépendances suivantes:
- un compilateur C (gcc ou clang)
- la bibliothèque ncurses et ses en-têtes de développement, préférablement la version wide-characters (cherchez un paquet dont le nom ressemble à libncursesw et un autre ressemblant à libncursesw-dev)
- gpm (facultatif; pour utiliser la souris dans un tty)
- git et un programme install compatible BSD (facultatifs; seulement pour suivre les instructions)

Suivez les instructions suivantes pour installer correctement le programme:

1. Clonez ce repo
```bash
git clone https://github.com/nico64-64/menu-terminal.git
cd menu-terminal
```

2. Compilez le programme
```bash
gcc menu.c -o menu -lncursesw
```

3. Installez le fichier de configuration donné en exemple
```bash
install -vdm755 ~/.config/menu
install -vm644 config.exemple ~/.config/menu/config
```

4. Installez le programme lui-même
```bash
sudo install -vm755 -o0 -g0 menu /usr/bin
```

## Configuration

Ce menu est facilement configurable via un fichier de configuration (normalement `~/.config/menu/config`) qui peut être modifié par l'utilisateur afin d'ajouter ou d'enlever des entrées du menu.

Un exemple d'un tel fichier est le fichier config.exemple fourni avec ce programme. Si vous avez suivi les instructions d'installation, il se trouve déjà au bon endroit et vous n'avez plus qu'à le personnaliser selon vos besoins. La syntaxe est expliquée au début du fichier.

## Usage

Vous pouvez démarrer ce menu depuis votre terminal avec la commande `menu`. Vous pouvez lui fournir quelques options de démarrage, dont la liste est affichée lorsque vous entrez `menu -?`. S'il y a une ou plusieurs options que vous voulez toujours utiliser, vous pouvez vous définir un alias dans votre ~/.bashrc ou ~/.zshrc.

Un texte d'aide est également disponible en entrant 'A' une fois le menu ouvert.

Vous pouvez entrer une commande spéciale en entrant 'C' ou '/' une fois le menu ouvert.

## Limitations

Ce programme ne vérifie pas si toutes les entrées tiennent dans le terminal, donc assurez-vous d'ajuster le nombre d'entrées (dans le fichier de configuration) ou l'espacement entre celles-ci en conséquence (par exemple, `menu -e 0` n'affichera aucune ligne vide entre les entrées, doublant le nombre d'entrées possible sans déborder).

## Notes

Les options "twin-on" et "twin-off" du fichier de configuration peuvent être utilisées pour vérifier si ce menu est démarré depuis un terminal twin (https://github.com/cosmos72/twin).

L'option "fbterm-off" vérifiera si le terminal depuis lequel ce programme est démarré est une instance de fbterm (https://wiki.archlinux.org/title/Fbterm), mais cela nécessite que vous créiez un script qui settera la variable `$FBTERM` de votre shell en même temps que vous démarrez fbterm. Ce menu assume également que fbterm est utilisé pour afficher une image en arrière-plan, ce qui explique pourquoi si le programme détecte la variable `$FBTERM`, il affichera un fond gris derrière le texte des entrées du menu. Si vous utilisez fbterm sans afficher d'image en arrière-plan, ce flag est inutile.
