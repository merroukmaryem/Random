#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <math.h>
#include<time.h>


/******Réaliser par Zaynab Douh et Maryem Merrouk**********/

// la taille de la table
#define TAILLE 2097152
//nombre de processus fils
#define NbDeFils 7
// le nombre de fois un processus appel rand()
#define NR (1000 * TAILLE ) / NbDeFils

// nombre d'entrées a la table globale
#define NbAllerRetour 100
sem_t *semaphore; // Déclaration du sémaphore
sem_t *semaphore2;

  /*Cette fonction permet de créer une mémoire partagée entre les processus*/

  /* elle retourne Un pointeur vers le tableau partagé*/

int *Memoire_Partage()
{
    int *tab = mmap(NULL, sizeof(int) * TAILLE ,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (tab != MAP_FAILED)
        return tab;

    perror("Erreur de creation de la mémoire partage");
    exit(EXIT_FAILURE);
}

/*Cette méthode permet d'initialiser un tableau à 0*/
 /* tab : Un tableau d'entiers */
void InitialiserTable(int *tab)
{
    for (int i = 0; i < TAILLE ; i++)
    {
        tab[i] = 0;
    }
}

/* Cette fonction permet d'allouer dynamiquement un tableau bidimentionnel*/

/* retourne Un pointeur vers le tableau alloué*/
int *AllouerTable()
{
    int *tab = (int *)malloc(TAILLE  * sizeof(int *));
    
    return tab;
}

// Fonction pour libérer un tableau bidimensionnel
void LibererTableau(int *tab) {
    free(tab);
}
/*Cette fonction permet d'afficher les valeurs générées du tableau si besoin*/
 /* tab : Un tableau d'entiers*/
void AfficherTable(int *tab)
{
    for (int i = 0; i < TAILLE ; i++)
    {
        printf(" %d", tab[i]);
    }
    printf("\n\n");
}

/*Cette méthode permet de faire la somme de tous les éléments du tableau*/
 /* tab : Un tableau d'entiers*/
 /*retourne La somme des éléments du tableau*/
unsigned long long Somme(int *tab)
{
    unsigned long long somme = 0;
    for (int i = 0; i < TAILLE ; i++)
    {
        somme += tab[i];
    }
    return somme;
}
/******************************************la somme est utilisée pour calculer la moyenne **********************/
/* Cette méthode permet de retourner la moyenne à partir de la somme déjà calculée*/
/* somme : La somme des éléments du tableau*/
/*retourne La moyenne */
double Moyenne(unsigned long long somme)
{
    return somme / (double)TAILLE ;
}
/*****************************************la moyenne est utilisée pour calculer la variance**********************/
/*Cette méthode renvoie la variance à partir de la moyenne*/
/* moyenne : La moyenne*/
/*tab : Un tableau d'entiers*/
/*retourne La variance*/
double Variance(double moyenne, int *tab)
{
    double variance = 0;
    for (int i = 0; i < TAILLE ; i++)
        variance += pow((tab[i] - moyenne), 2);
    return variance /TAILLE  ;
}
/*****************************************la variance est utilisée pour calculer l'écart type**********************/
/* Cette méthode permet de calculer l'écart type à partir de la variance*/
/*variance : La variance*/
/*retourne L'écart type*/
double Ecartype(double variance)
{
    return sqrt(variance);
}
/*****************************************l'écart type et la moyenne sont utilisés pour calculer le coefficient de variation**********************/
/*Cette méthode permet de calculer le coefficient de variation à partir d'un écart type et une moyenne calculés*/
 /*ecarType : L'écart type*/
 /* moyenne : La moyenne*/
/*retourn Le coefficient de variation*/
double CoeffVariation(double ecarType, double moyenne)
{
    return ecarType / moyenne;
}

/************************************on va utiliser coefficient pour étudier l'équilibrage de la fonction rand() de la bibliothéque C******************************************/
int main()
{
pid_t pid;
int *tab=NULL;
int *tab2=NULL;
int  *tab_globale=NULL;
int  *tab_globale2=NULL;

//allocation d'une table bidimentionnelles chaque processus utilisera une ligne

//reservation de mémoire
tab_globale=Memoire_Partage();
tab_globale2=Memoire_Partage();
//initialisation de la table globale
InitialiserTable(tab_globale);
InitialiserTable(tab_globale2);
//détruit sémaphore
sem_unlink("semaphore");
sem_unlink("semaphore2");
//ouvrir le semaphore
semaphore=sem_open("semaphore",O_CREAT |O_EXCL,0,1);
semaphore2=sem_open("semaphore2",O_CREAT |O_EXCL,0,1);
if(semaphore==SEM_FAILED)
{
    perror("Semaphore failed");
    exit(EXIT_FAILURE);
}
if(semaphore2==SEM_FAILED)
{
    perror("Semaphore 2 failed");
    exit(EXIT_FAILURE);
}


for(int i=0;i<NbDeFils;i++)
{pid=fork();  // création des processus
    if(pid==-1) //fork erroné
        {
            perror("fork errone");
            return -1;
        }
    if(pid==0) // processus fils
        {for(int z=0;z<NbAllerRetour;z++){ //boucler pour faire des aller retour vers la table globale plusieurs fois (sans ecraser les valeurs déjà ajouter par les processus )
        
        tab=AllouerTable();
        tab2=AllouerTable();
        InitialiserTable(tab);
	InitialiserTable(tab2);
            srand(getpid()); //initialiser le générateur de nombres aléatoires utilisé par la fonction rand() en C
            srand48(getpid());

//Chaque processus remplit sa table en incrémentant la valeur de la case dont l'indice est obtenu par la fonction rand()
 for (u_int64_t  j = 0; j < NR; j++) {
              u_int32_t indice = rand() % TAILLE ;  // Indice aléatoire
                tab[indice]++;
                }

for (u_int64_t  l = 0; l < NR; l++) {
    //unsigned int seed =TAILLE ;
           u_int32_t indice2=  lrand48() % TAILLE  ;

                  tab2[indice2]++;   // Incrémentation de la case correspondante
            }

        // bloquer le sémaphore avant de mettre à jour la mémoire partagée
            sem_wait(semaphore);
           
                for(int m=0;m<TAILLE ;m++){
                tab_globale[m] +=tab[m];
            }
             sem_wait(semaphore2);
            
                for(int m=0;m<TAILLE ;m++){
                tab_globale2[m] +=tab2[m];
            }
       // libérer le sémaphore après la mise à jour
        sem_post(semaphore);
          sem_post(semaphore2);
  
     
        return 0;}}}

//attendre tous les fils
for(int i=0;i<NbDeFils;i++)
{wait(NULL);}






// Fermer le sémaphore pour libérer les ressources associées
sem_close(semaphore);
sem_close(semaphore2);
// Supprimer le sémaphore
sem_unlink("semaphore");
sem_unlink("semaphore2");
// Etudier l'équilibrage de la fonction rand()

 printf("\n\n ****************************************************");

    printf("\n *****  L'EQUILIBRAGE DE LA FONCTION RAND()  ******");

    printf("\n ****************************************************");

    unsigned long long somme = Somme(tab_globale);
    printf("\n . LA SOMME  = %lld\n", somme);

    double moy = Moyenne(somme);
    printf("\n . LA MOYENNE  = %lf\n", moy);

    double var = Variance(moy, tab_globale);
    printf("\n . LA VARIANCE =  %lf\n", var);

    double ecartype = Ecartype(var);
    printf("\n . L'ECARTYPE %lf\n ", ecartype);

    double coeffvariation = CoeffVariation(ecartype, moy);
    printf("\n . COEFFICIENT DE VARIATION %lf%c\n ", coeffvariation * 100, 37);   //37:code asccii de pourcentage

    // On vérifie si le coefficient de variation est inférieur à 5,
    // autrement dit le coefficient est faible
    // Donc la fonction rand() est equilibré.
    if (coeffvariation < 5.0)
    {
        printf("\n . LA FONCTION rand() du C EST EQUILIBREE\n");
    }
    else
    {
        printf("\n . LA FONCTION rand() du C N'EST PAS EQUILIBREE\n");
    }

    printf("\n ****************************************************\n");


     printf("\n\n ****************************************************");

    printf("\n *****  L'EQUILIBRAGE DE LA FONCTION lrand48()  ******");

    printf("\n ****************************************************");

    unsigned long long somme2 = Somme(tab_globale2);
    printf("\n . LA SOMME  = %lld\n", somme2);

    double moy2 = Moyenne(somme2);
    printf("\n . LA MOYENNE  = %lf\n", moy2);

    double var2 = Variance(moy2, tab_globale2);
    printf("\n . LA VARIANCE =  %lf\n", var2);

    double ecartype2 = Ecartype(var2);
    printf("\n . L'ECARTYPE %lf\n ", ecartype2);

    double coeffvariation2 = CoeffVariation(ecartype2, moy2);  
    printf("\n . COEFFICIENT DE VARIATION %lf%c\n ", coeffvariation2 * 100, 37);   //37:code asccii de pourcentage

    // On vérifie si le coefficient de variation est inférieur à 5,
    // autrement dit le coefficient est faible
    // Donc la fonction rand() est equilibré.
    if (coeffvariation2 < 5.0)
    {
        printf("\n . LA FONCTION lrand48() du C EST EQUILIBREE\n");
    }
    else
    {
        printf("\n . LA FONCTION lrand48() du C N'EST PAS EQUILIBREE\n");
    }

    printf("\n ****************************************************\n");
  
// Libérer la mémoire allouée pour les tables
LibererTableau(tab);
LibererTableau(tab2);
// Libérer la mémoire allouée pour la table globale
munmap(tab_globale, sizeof(int) * TAILLE );
munmap(tab_globale2, sizeof(int) * TAILLE );
    return 0;
} 
/*********************Les résultats d'uniformité pour rand() et lrand48() *******************************/
/*
 ****************************************************
 *****  L'EQUILIBRAGE DE LA FONCTION RAND()  ******
 ****************************************************
 . LA SOMME  = 2097151994

 . LA MOYENNE  = 999.999997

 . LA VARIANCE =  999.239287

 . L'ECARTYPE 31.610746
 
 . COEFFICIENT DE VARIATION 3.161075%
 
 . LA FONCTION rand() du C EST EQUILIBREE

 ****************************************************


 ****************************************************
 *****  L'EQUILIBRAGE DE LA FONCTION lrand48()  ******
 ****************************************************
 . LA SOMME  = 2097151994

 . LA MOYENNE  = 999.999997

 . LA VARIANCE =  991.345406

 . L'ECARTYPE 31.485638
 
 . COEFFICIENT DE VARIATION 3.148564%
 
 . LA FONCTION lrand48() du C EST EQUILIBREE

 ****************************************************
*/
