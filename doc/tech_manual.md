---
lang: fr-FR
title: Manuel Technique - Http-miniserv
subtitle: Manuel technique du mini serveur Http
author:
    - Rémi Labbé
date: Mars 2022
documentclass: report
toc: true
fontsize: 12
mainfont: Source Code Pro Medium
monofont: mononoki Nerd Font
---

# Introduction

Le manuel technique sert a décrire les différents choix et contraites qui ont été
 rencontrés pendant le développement de ce projet. La spécification des fonctions
 et structures a été faite dans le code lui même.

# Server

The server is started in the content folder to block access the source code.
It open a listenning socket on the port specified in the `config.h` file
The server is multithreaded, it starts a new thread for each request received.
The thread destroy itself when it has sent the file.

## Sockets

The server is using TCP sockets developped as a independent library named *tcpsockets*.
It makes it easy to create a listenning sockets (waiting for request) and clients sockets
(for sending request to any server not only this one).

Each funtion is documented directly in the code for the "How to use" part of this manual.

The server is started by default on port 8080 to avoid the need for sudo.
PORTS under 1024 are reserved to root, so port 80, the default for http isn't available.

## Regex

I developped a little regex library used by the server to check the format of incomming requests.
It provides 2 function, one for testing a string against a REGEX, the other one to extract the 
matching group of the tested string.

Macros defined in the format.h file should be changed accordingly to your usage if you use this library alone.

# Limitations

L'envoie de requetes via telnet est limite dans la version actuelle du projet.
Telnet envoyant les lignes de l'entete de la requete une a une il faudrait changer l'implementation 
de la reception de la requetes pour remplir un buffer jusqu'a recontrer une ligne vide et plutot que n'attendre qu'un 
paquet par client.

# Conclusions

La decouverte et l'implementation d'un serveur etait une activite enrichisante et plaisante a realiser. Je pense 
continuer a y reflechir sur mon temps libre pour implementer plus de fonctionnalite. Le fonctionnement du PHP me semble 
entre autres plutot interessant.
