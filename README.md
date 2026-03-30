## Contexte
Projet de fin de semestre effectué dans le cadre du cours de CGDI (Computer Graphics and Digital Images) de l'ENS de Lyon. Ce projet a été effectué seul et vise à construire un logiciel de rasterization efficace et cross-plateform.


## Choix du langage 
Dans les TP du cours on utilise 2 langages: C++ et python, donc je voulais prendre l'un de ces deux langage pour mon projet. Puisque la rasterization demande une grande efficacité, le choix de c++ face à python paraît logique pour optimiser le plus possible les différents paramètres non-algorithmiques.

## Fonctionnement du projet
Le projet peut être découpé est deux parties:
- L'implémentation de la rasterisation permettant de visualiser le simulateur sur une fenêtre de taille prédéfinie
- L'implémentation d'un algorithme de re-fenetrage de la page du simulateur pour lui permettre d'être plus grande/petite sans pour autant changer la résolution du simulateur.

Cette approche permet d'avoir un temps de calcul qui ne dépend pas de la taille choisie par l'utilisateur mais seulement de la taille prédéfinie du simulateur. Ainsi, en cas de soucis de vitesse de calculs, cette taille prédéfinie peut être réduite sans changer la taille réelle de l'affichage.
Tout le soucis de la deuxième partie est donc d'afficher au mieux le simulateur en essayant d'alterer le moins possible l'apparence visuelle.

## Optimisation
Pour des raisons de simplicité, aucune optimisation sera effectué pour que le projet puisse tourner sur un GPU. Il ne fonctionne par défaut seulement sur le CPU de l'ordinateur.
En revanche, du multi-threading est implémenté pour prendre partie des différents coeurs du processeur de l'utilisateur.

## Utilisation
L'implémentation d'un makefile permet de lancer le projet avec la commande suivante (compilation + lancement de l'exécutable):
> make && ./main


