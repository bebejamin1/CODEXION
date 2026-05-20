# CODEXION

cc -pthread main.c

premier truc a faire cest le parsing prendre les bonnes valeurs
avoir un monitor routine et un codeur routine

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


