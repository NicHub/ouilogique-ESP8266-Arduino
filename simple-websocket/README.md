SIMPLE-WEBSOCKET.INO
====================

Ce programme permet de communiquer avec un ESP8266 avec le protocole WebSocket. L’ESP8266 se connecte au réseau WiFi qu’on lui indique dans le programme et il est ensuite possible de communiquer avec lui avec un navigateur web sur le même réseau.

<p><a href="https://github.com/NicHub/ouilogique-ESP8266-Arduino/raw/master/simple-websocket/images/simple-websocket-screenshot-01.png" target="_blank"><img src="https://github.com/NicHub/ouilogique-ESP8266-Arduino/raw/master/simple-websocket/images/simple-websocket-screenshot-01.png" alt="Capture d’écran simple-websocket.ino" width="400px;" /></a></p>

# Matériel requis

- Un module ESP8266. De préférence [un modèle 12E avec deux LED, comme les Amica](http://ouilogique.com/NodeMCU_esp8266_amica/).

# Logiciels requis

- L’IDE Arduino 1.6.9

## Configuration de l’IDE Arduino

- Installer la bibliothèque pour programmer l’ESP8266 dans l’IDE Arduino. Dans les préférences de l’IDE, sous “URL de gestionnaire de cartes supplémentaires”, ajouter le lien suivant : `http://arduino.esp8266.com/stable/package_esp8266com_index.json`.
- Dans le menu `Outils/Type de carte/Gestionnaire de carte` installer la famille `esp8266 by ESP8266 Community`.
- Changer le type de carte dans le menu `Outils/Type de carte` et choisir `NodeMCU 1.0 (ESP-12E Module)`.
- Dans le menu `Croquis/Inclure une Bibliothèque/Gérer les bibliothèques`, inclure la librairie `WebSockets` de Markus Sattler.
- Installer le gestionnaire de téléchargement *esp8266fs*. Voir <https://github.com/esp8266/arduino-esp8266fs-plugin/>
	- Télécharger [le fichier zip](https://github.com/esp8266/arduino-esp8266fs-plugin/releases/tag/0.2.0)
	- Créer le dossier `~/Documents/Arduino/tools/` s’il n’existe pas.
	- Extraire le contenu du fichier zip et le copier dans le répertoire `~/Documents/Arduino/tools/`. Au final, il n’y a qu’un seul fichier `~/Documents/Arduino/tools/ESP8266FS/tool/esp8266fs.jar`.
	- Redémarrer l’IDE Arduino et le menu `Outils` doit maintenant contenir la commande `ESP8266 Sketch Data Upload`.
	- Lors de l’utilisation d’*esp8266fs*, il faut que le moniteur série de l’IDE soit fermé, sinon une erreur est générée.

Il y a quelques précisions supplémentaires [sur mon blog](http://ouilogique.com/NodeMCU_esp8266/#programmation-en-arduino-c).



# Préparation

## Type de module

Ce programme fonctionne sur ESP-01 et ESP-12E. Suivant le type de module utilisé, il faut adapter les lignes suivantes (exemple pour ESP-01) :

ligne 26 de `ws_functions.h`, indiquer `#define ESP_MODULE_TYPE 'ESP-01'`

ligne 7 de `websocket.js`, indiquer `var ESP_MODULE_TYPE = 'ESP-01'`


## Paramètres WiFi

Créer un fichier `WifiSettings.h` à la racine du projet et contenant les informations suivantes :

    const char* ssid     = "***";
    const char* password = "***";

## Compiler et télécharger le programme

Tout est dans le titre... (CMD + U ou CTRL + U)

## Téléchargement des fichiers du serveur web de l’ESP8266

Lors du premier téléchargement, il est préférable d’utiliser le gestionnaire *esp8266fs* qui permet de télécharger tous les fichiers en une fois. Comme il est très lent, il est préférable d’utiliser la commande `curl` dans le terminal `bash` lorsqu’on ne veut télécharger qu’un fichier à la fois. Cette commande ne fonctionnera que lorsque l’ESP sera flashé avec le programme `simple-websocket.ino`.

**Exemples de commande :**

> Note : la commande `curl` est beaucoup plus rapide avec l’adressse IP qu’avec le nom de domaine.

    cd data
    ip=$(ping -c 1 esp8266.local | gawk -F'[()]' '/PING/{print $2}')
    echo $ip
    curl                                             \
        -F "file=@img1.jpg"        http://$ip/upload \
        -F "file=@img2.jpg"        http://$ip/upload \
        -F "file=@index.html"      http://$ip/upload \
        -F "file=@logo.png"        http://$ip/upload \
        -F "file=@style.css"       http://$ip/upload \
        -F "file=@websocket.js"    http://$ip/upload



# Utilisation

Lors du démarrage, l’ESP indique son adresse IP dans la console série. Il est également possible de connaître son adresse IP avec la commande bash :

	ping -c1 esp8266.local

Ensuite on peut utiliser l’interface web de l’ESP avec un navigateur internet à l’adresse :

http://192.168.1.131

Il est également possible d’utiliser le nom de domaine configuré dans le programme `simple-websocket.ino`. Mais certains navigateurs, comme Safari, n’arrivent pas à résoudre l’adresse :

http://esp8266.local

Si tout à fonctionné normalement, le navigateur affiche le fichier index.html :

![Capture d’écran simple-websocket.ino](https://github.com/NicHub/ouilogique-ESP8266-Arduino/raw/master/simple-websocket/images/simple-websocket-screenshot-01.png)

