# CODEXION

QUESTION
multiplet ? Stack ? heap ? data race ? deadlock ?





POXIX interface standart pour manipuler les thread dans la bibliotech pthread.h

cc -pthread main.c

premier truc a faire cest le parsing prendre les bonnes valeurs
avoir un monitor routine et un codeur routine


int pthread_create(pthread_t *restrict thread,
                          const pthread_attr_t *restrict attr,
                          void *(*start_routine)(void *),
                          void *restrict arg);

thread : un pointeur vers une variable de type pthread_t pour stocker l’identifiant du thread qu’on va créer.
attr : un argument qui permet de changer les attributs par défaut du nouveau thread lors de sa création. Ceci va au-delà de la portée de cet article, et en général il suffira d’indiquer NULL ici.
start_routine : la fonction par laquelle le thread commence son exécution. Cette fonction doit avoir pour prototype void *nom_de_fonction_au_choix(void *arg);. Lorsque le thread arrive à la fin de cette fonction, il aura terminé toutes ses tâches.
arg : le pointeur vers un argument à transmettre à la fonction start_routine du thread. Si l’on souhaite passer plusieurs paramètres à cette fonction, il n’y a pas d’autre choix que de lui renseigner ici un pointeur vers une structure de données.


 int pthread_join(pthread_t thread, void **retval);

thread : l’identifiant du thread qu’on attend. Le thread spécifié ici doit être joignable (c’est à dire non détaché - voir ci-dessous).
retval : un pointeur vers une variable qui peut contenir la valeur de retour de la fonction routine du thread (la fonction start_routine qu’on a fournie lors de la création du thread). Si on n’a pas besoin de cette valeur, on peut simplement renseigner NULL.
pthread_join renvoie 0 en cas de succès, ou dans le cas contraire, un code erreur









DATA RACE aled (situaiton de competition)
ce produit quand le programme depend de la progression ou du timing 
dautres evenements incontrollable 

compilation -fsanitize=thread -g
permet de chopper lerreur -> “WARNING: ThreadSanitizer: data race”.

MUTEX (exclusion mutuelle)
regule lacces au donnee et empeche que les ressources partager soit utiliser
en meme temps

declarer un mutex 
pthread_mutex_t    mutex;
initialiser 

int pthread_mutex_init(pthread_mutex_t *mutex,
                        const pthread_mutexattr_t *mutexattr);


verrou deverrou
int pthread_mutex_lock(pthread_mutex_t *mutex); // Verrouillage
int pthread_mutex_unlock(pthread_mutex_t *mutex); // Déverrouillage

detruire
int pthread_mutex_destroy(pthread_mutex_t *mutex);


deadlock mutex bloquer 

Gérer les interblocages
Il y a plusieurs façons de gérer les interblocages comme ceux-ci. On peut entre autres :

les ignorer, mais seulement si on peut prouver qu’ils ne surviendront jamais. Par exemple, quand les intervalles de temps entre les demandes d’accès aux ressources partagées sont très longues.
les corriger quand ils surviennent en tuant un thread ou en redistribuant les ressources, par exemple.
les prévenir et les corriger avant qu’ils ne surviennent.
les éviter en imposant un ordre strict d’acquisition des ressources. C’est la solution à notre exemple précédent : les threads devraient demander tous deux le lock_1 en premier.
les éviter en forçant un thread a relâcher une ressource avant d’en demander de nouvelles ou de renouveler sa demande.
Il n’y a pas de “meilleure” solution pour gérer tous les cas d’interblocage. La méthode de gestion des interblocages doit se faire au cas par cas, selon la situation.



STRUCTURE COMPREHENSION


double = 8 octet
char = 1 octet
entier = 4 octet

taille de la structure depend de la memoire si elle est stocker en 8 octer 
elle allouera forcement la taille 8 par 8

_Alignof avec <stdalign.h> pour les contrainte dalignement vue ci dessus



TESTER LE PROGRAMME

Le drapeau -fsanitize=thread -g qu’on ajoute au moment de la compilation. L’option -g permet d’afficher les numéros de ligne qui ont produit l’erreur.
L’outil de détection d’erreurs de thread Helgrind avec lequel on peut exécuter notre programme, comme ceci : valgrind --tool=helgrind ./programme.
L’outil de détection d’erreurs de threads DRD, qu’on lance aussi au moment de l’exécution comme ceci : valgrind --tool=drd ./programme.
Attention, valgrind et -fsanitize=thread ne s’entendent pas du tout et ne doivent pas s’utiliser ensemble !

Comme toujours, il ne faut pas non plus oublier de vérifier les fuites de mémoire avec -fsanitize=address et valgrind tout court !