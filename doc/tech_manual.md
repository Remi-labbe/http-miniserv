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

Le manuel technique sert à décrire les différents choix et contraites qui ont été
 rencontrés pendant le développement de ce projet. La spécification des fonctions
 et structures à été faite dans le code lui même.

# Serveur

Le serveur est lancé dans le dossier "content" pour bloquer l'acces au code source.
En effet on ne peut pas remonter plus haut (dans l'arborescence des fichiers) 
que le dossier dans lequel l'executable est lancé.
Il ouvre une socket d'écoute sur le port specifié dans le fichier `config.h`.
Il lancera un nouveau thread pour chaque nouvelle requete afin de pouvoir traiter plusieurs clients en meme temps.
Ces threads seront automatiquement détruit quand leur routine sera termiée.

## Sockets

Ce serveur à été implementé en utilisant les sockets TCP qui sont definis dans un module independant *tcpsockets*.
Celui ci requiert cependant le module *internetaddr* pour fonctionner.
Il rend facile la création de sockets d'écoutes (attendant des requetes) et des socket clients (envoyant des requetes 
à une adresse).

L'utilité de chaque fonction et son utilisation sont specifiées dans le fichier `tcp_socket.h`

La socket d'écoute du serveur est ouverte par default sur le port 8080 pour eviter d'avoir besoin des droit sudo pour 
démarrer le serveur.
Les port inferieurs à 1024 sont en effet réservé à l'utilisateur ROOT, le port par default du protocole: 80, en fait donc
partie.

## Regex

J'ai développé un petit module pour utiliser les regex pour vérifier le format des requetes recues par le serveur.
Il fournit 2 fonctions, une pour tester une chaine de charateres avec un paterne défini et l'autre pour extraire les 
groupes de cette derniere.

les constantes présentes dans le fichier `format.h` definisse le comportement de ces fonctions.

# Limitations

L'envoie de requetes via telnet est limité dans la version actuelle du projet.
Telnet envoyant les lignes de l'entete de la requete une à une il faudrait changer l'implementation 
de la récéption de la requetes pour remplir un buffer jusqu'à recontrer une ligne vide et plutot que n'attendre qu'un 
paquet par client.

# Conclusions

La découverte et l'implémentation d'un serveur était une activité enrichisante et plaisante à realiser. Je pense 
continuer à y reflechir sur mon temps libre pour implémenter plus de fonctionnalités. Le fonctionnement du PHP me semble 
entre autres plutot interessant.
