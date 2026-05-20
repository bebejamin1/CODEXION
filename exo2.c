#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

struct t_cuisine
{
    int             beurre_dispo; // 0 = non, 1 = oui
    pthread_mutex_t *cadenas;
    // --- À TOI DE JOUER (Étape 1) ---
    // Ajoute un pointeur vers une variable pthread_cond_t appelée 'cloche'
    pthread_cond_t	*cloche;
};

void *recette_du_commis(void *arg)
{
    struct t_cuisine *cuisine = (struct t_cuisine *)arg;

    printf("   [COMMIS] Je veux faire le gâteau. Je vérifie le frigo...\n");
    
    pthread_mutex_lock(cuisine->cadenas); // Je prends la porte du frigo

    // Si le beurre n'est pas là, je dois m'endormir
    if (cuisine->beurre_dispo == 0)
    {
        printf("   [COMMIS] Pas de beurre ! Je m'endors devant le frigo...\n");
        // --- À TOI DE JOUER (Étape 2) ---
        // Utilise pthread_cond_wait pour t'endormir. 
        // Rappel : ça prend la cloche ET le cadenas !
        pthread_cond_wait(cuisine->cloche, cuisine->cadenas);
    }

    // Si on arrive ici, c'est qu'on a été réveillé ET qu'on a repris le cadenas
    printf("   [COMMIS] Ah ! Il y a du beurre ! Je fais mon gâteau.\n");
    cuisine->beurre_dispo = 0; // Je prends le beurre

    pthread_mutex_unlock(cuisine->cadenas); // Je ferme la porte du frigo
    return (NULL);
}

int main(void)
{
    pthread_t       mon_commis;
    pthread_mutex_t mon_cadenas;
    // --- À TOI DE JOUER (Étape 3) ---
    // Déclare une variable pthread_cond_t (ma_cloche) et initialise-la avec init
    pthread_cond_t	ma_cloche;
	pthread_cond_init(&ma_cloche, NULL);
    
    struct t_cuisine cuisine;
    cuisine.beurre_dispo = 0;
    
    pthread_mutex_init(&mon_cadenas, NULL);
    cuisine.cadenas = &mon_cadenas;
    // --- À TOI DE JOUER (Étape 4) ---
    // Fais pointer cuisine.cloche vers &ma_cloche
	cuisine.cloche = &ma_cloche;
    

    pthread_create(&mon_commis, NULL, recette_du_commis, &cuisine);

    // Le Chef fait ses courses
    sleep(2);
    
    printf("[CHEF] Je reviens des courses !\n");
    pthread_mutex_lock(&mon_cadenas);
    cuisine.beurre_dispo = 1; // Le Chef met le beurre
    printf("[CHEF] J'ai mis le beurre, je réveille le commis !\n");
    
    // --- À TOI DE JOUER (Étape 5) ---
    // Utilise pthread_cond_broadcast pour sonner la cloche et réveiller le commis !
	pthread_cond_broadcast(&ma_cloche);

    pthread_mutex_unlock(&mon_cadenas); // Le Chef lâche le frigo pour que le commis puisse se servir

    pthread_join(mon_commis, NULL);

    pthread_mutex_destroy(&mon_cadenas);
    // --- À TOI DE JOUER (Étape 6) ---
    // Détruis ta cloche avec pthread_cond_destroy
	// pthread_cond_destoy;

    printf("[CHEF] Gâteau terminé.\n");
    return (0);
}