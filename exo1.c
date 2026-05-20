#include <stdio.h>
#include <unistd.h>   // Pour la fonction sleep()
#include <pthread.h>  // Pour les threads

struct t_cuisine
{
    int *sel_dans_la_soupe;
    pthread_mutex_t *cadenas;
};

void *recette_du_commis(void *arg)
{
    struct t_cuisine *cuisine = (struct t_cuisine *)arg;

    printf("   [COMMIS] Je commence mon travail...\n");
    int i = 0;
    while (i <= 1000000)
    {
        pthread_mutex_lock(cuisine->cadenas);
        *(cuisine->sel_dans_la_soupe) += 1;
        i++;
        pthread_mutex_unlock(cuisine->cadenas);
    }
    printf("   [COMMIS] J'ai fini !\n");
    
    return (NULL);
}

int main(void)
{
    pthread_mutex_t mon_cadenas;
    pthread_mutex_init(&mon_cadenas, NULL);
    pthread_t   mon_commis;
	pthread_t   mon_commis2;
    int         soupe = 0;
    struct      t_cuisine cuisine;
    cuisine.sel_dans_la_soupe = &soupe;
    cuisine.cadenas = &mon_cadenas;

    printf("[CHEF] Le restaurant ouvre.\n");
    printf("[CHEF] J'embauche le commis et je le lance !\n");

    pthread_create(&mon_commis, NULL, recette_du_commis, &cuisine);
	pthread_create(&mon_commis2, NULL, recette_du_commis, &cuisine);

    printf("[CHEF] Le commis travaille. Moi, je l'attends...\n");

    pthread_join(mon_commis, NULL);
    pthread_join(mon_commis2, NULL);

    pthread_mutex_destroy(&mon_cadenas);

    printf("%d\n", soupe);

    printf("[CHEF] Tout le monde a fini, je ferme le restaurant.\n");
    return (0);
}
