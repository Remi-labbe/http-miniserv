---
lang: fr-FR
title: Manuel Utilisateur - Http-miniserv
subtitle: Manuel utilisateur du mini serveur Http
author:
    - Rémi Labbé
date: Mars 2022
documentclass: report
fontsize: 12
mainfont: Source Code Pro Medium
monofont: mononoki Nerd Font
---

## utilisation

### Compiler le projet

```bash
make
```

### Demarrer/Arreter le Serveur

```bash
./server
```

Les fichier auquel le serveur accede doivent etre place dans le dossier *content*.

Il est lance par default sur le port 8080 pour eviter d'avoir besoin des droits sudo.
Le port d'ecoute est modifiable dans le fichier `config.h`.

Le serveur peut etre arrete par les signaux "C-c" ou "C-\".

### Possibilitees

Ce "mini-serveur" prend en charge les types mimes communs, seulement les requetes GET.
On peut trouver les plus communs: html, image, videos, javascript...

### Acces client

`localhost:PORT/filename.ext`

- Via un navigateur internet.
- teste avec curl, wget et postman.
