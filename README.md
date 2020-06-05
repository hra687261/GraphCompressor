# CompressGraph
[PSTL]Project

Pour la version la plus récente :  
Execution :   

Construction d'un graphe compréssé et écriture dans la mémoire :  

./exec_writeCAL 

param 1 : id de la structure de données, (1 pour la adjlist avec la méthode des gaps, 2 pour la méthode des copylists (et les noeuds restants codés avec la méthode des gaps), 3 pour la méthode des copyblocks, 4 pour la méthode des intervalles avec un tableau cd[i]=bit à partir duquel la lsite d'adjascence de i est codée, 5 pour la méthode des intervalles en utilisant le "jump", le tableau "cd" étant plus petit cette méthode est la plus éfficace ) .  

param 2 : chemin vers les fichier contenant la liste de liens.  

param 3 : nombre de ligne à ignorer dans le début du fichier, si il y'a des commentaires.  

param 4 : chemin vers le fichier de sortie (il sera crée si il existe pas) contenant le graphe compéssé.  

param 5 : fonction de compression utilisée (1 pour gamma, 2 pour delta, 3 pour nibble, 4 pour zeta).  

param 6 : si la fonction de compression choisie est zeta, il faut ajouter le "shrinking parameter" k.  

param 7 : la taille de la "fenetre" (window size) pour la compression avec les copylists et copyblocks.  

param 8 : la taille maximale d'une chaine de réference.  

param 9 : la taille minimale d'un intervalle pour la compression avec les intervalles.  

param 10 : la longeur du "jump", pour la compression avec les intervalles.   

Lecture d'un graphe compréssé et exécution de BFS;

./exec_readCAL  

param 1 : id de la structure de données.

param 2 : chemin vers le fichier du graphe compréssé.   

param 3 : le premier noeud du parcours en largeur.  

param 4 : booléen, vaut 1 si on veux afficher la liste des noeuds du parcours en largeur sinon on affiche juste le nombre de noeud du parcours.  

