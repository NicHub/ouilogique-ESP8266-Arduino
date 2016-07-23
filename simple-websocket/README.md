simple-websocket
================

Ce programme permet de communiquer avec un ESP8266 avec le protocole WebSocket. L’ESP8266 se connecte au réseau WiFi qu’on lui indique dans le programme et il est ensuite possible de communiquer avec lui avec un navigateur web sur le même réseau.

# Matériel requis

- Un module ESP8266. De préférence un modèle 12E avec deux LED, comme les Amica.

# Logiciels requis

- L’IDE Arduino 1.6.9

## Configuration de l’IDE Arduino

Les instructions détaillées se trouvent [sur mon blog](http://ouilogique.com/NodeMCU_esp8266/#programmation-en-arduino-c). En bref :

- Installer la bibliothèque pour programmer l’ESP8266 dans l’IDE Arduino. Dans les préférences de l’IDE, sous “URL de gestionnaire de cartes supplémentaires”, ajouter le lien suivant : `http://arduino.esp8266.com/stable/package_esp8266com_index.json`. Puis dans le menu `Outils/Type de carte/Gestionnaire de carte` installer la famille `esp8266 by ESP8266 Community`.
- Changer le type de carte dans le menu `Outils/Type de carte` et choisir `NodeMCU 1.0 (ESP-12E Module)`.
- Dans le menu `Croquis/Inclure une Bibliothèque/Gérer les bibliothèques`, inclure la librairie `WebSockets` de Markus Sattler.
- Installer le gestionnaire de téléchargement *esp8266fs*. Voir <https://github.com/esp8266/arduino-esp8266fs-plugin/>
	- Télécharger [le fichier zip](https://github.com/esp8266/arduino-esp8266fs-plugin/releases/tag/0.2.0)
	- Créer le dossier `~/Documents/Arduino/tools/` s’il n’esiste pas.
	- Extraire le contenu du fichier zip et le copier dans le répertoire `~/Documents/Arduino/tools/`. Au final, il n’y a qu’un seul fichier `~/Documents/Arduino/tools/ESP8266FS/tool/esp8266fs.jar`.
	- Redémarrer l’IDE Arduino et le menu `Outils` doit maintenant contenir la commande `ESP8266 Sketch Data Upload`.
	- Lors de l’utilisation d’*esp8266fs*, il faut que le moniteur série de l’IDE soit fermé, sinon une erreur est générée.




# Préparation

## Paramètres WiFi

Créer un fichier `WifiSettings.h` à la racine du projet et contenant les informations suivantes :

    const char* ssid     = "***";
    const char* password = "***";

## Compiler et télécharger le programme

Tout est dans le titre... (CMD + U ou CTRL + U)

## Téléchargement des fichiers du serveur web de l’ESP8266

Lors du premier téléchargement, il est préférable d’utiliser le gestionnaire *esp8266fs* qui permet de télécharger tous les fichiers en une fois. Comme il est très lent, il est préférable d’utiliser la commande `curl` dans le terminal `bash` lorsqu’on ne veut télécharger qu’un fichier à la fois. Cette commande ne fonctionnera que lorsque l’ESP sera flashé avec le programme `simple-websocket.ino`.

**Exemples de commande :**

	cd data
	curl -F "image=@index.html" http://192.168.1.131/upload
	curl -F "image=@websocket.js" http://192.168.1.131/upload
	curl -F "image=@img1.jpg" http://192.168.1.131/upload




# Utilisation

Lors du démarrage, l’ESP indique son adresse IP dans la console série. Il est également possible de connaître son adresse IP avec la commande bash :

	ping -c1 esp8266.local

Ensuite on peut utiliser l’interface web de l’ESP avec un navigateur internet à l’adresse :

http://192.168.1.131

Il est également possible d’utiliser le nom de domaine configuré dans le programme `simple-websocket.ino`. Mais certains navigateurs, comme Safari, n’arrivent pas à résoudre l’adresse.

http://esp8266.local


