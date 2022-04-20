#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

#define NRM "\x1B[1;0m"
#define RED "\x1B[1;31m"
#define GRN "\x1B[1;32m"
#define YEL "\x1B[1;33m"
#define BLU "\x1B[1;34m"
#define MAG "\x1B[1;35m"
#define CYN "\x1B[1;36m"
#define WHT "\x1B[1;37m"

typedef struct Person
{
    int id;
    char name[100];
    char zone[100];
    char occupied_zone[100];
    int zone_time;
    int time;
    int patience;
    int num_goals;
    int group;
    pthread_t Person_thread_id;
} Person;

typedef struct goal_chance
{
    int id;
    char team[100];
    int time;
    double probability;
    pthread_t goal_chance_id;
} goal_chance;

int groups[1000];
time_t seconds;
int capacity_H, capacity_A, capacity_N;
time_t starttime = 0;
Person persons[1000];
goal_chance team_chances[1000];
sem_t persons_sem[1000];
sem_t goal_chance_sem[1000];
sem_t performance_sem[1000];
pthread_mutex_t mutex;
int Filled_H[1000] = {0}, Filled_N[1000] = {0}, Filled_A[1000] = {0};
int count_H = 0;
int count_A = 0;
int wait_H = 0, wait_A = 0, wait_N = 0, goal_H = 0, goal_A = 0;
int waitH[1000], waitN[1000], waitA[1000], countH = 0, countA = 0, countN = 0;
int statuswait_H[1000] = {0}, statuswait_A[1000] = {0}, statuswait_N[1000] = {0};
int allocH[1000], allocN[1000], allocA[1000], alloc_countA = 0, alloc_countN = 0, alloc_countH = 0;
int actualH[1000], actualN[1000], actualA[1000], actual_countA = 0, actual_countN = 0, actual_countH = 0;
sem_t waitH_sem[1000], waitN_sem[1000], waitA_sem[100];
int X, num_groups, ticks[1000];

void init_semaphores()
{
    for (int i = 0; i < 1000; i++)
    {
        sem_init(&(persons_sem[i]), 0, 0);
    }
    for (int i = 0; i < 1000; i++)
    {
        sem_init(&(goal_chance_sem[i]), 0, 0);
    }
    for (int i = 0; i < 1000; i++)
    {
        sem_init(&(performance_sem[i]), 0, 0);
    }
    pthread_mutex_init(&mutex, NULL);
}

void *Teams_func(void *args)
{
    goal_chance *T = (goal_chance *)args;
    int time;
    time = (clock() - starttime) / CLOCKS_PER_SEC;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += T->time - 1;
    sem_timedwait(&persons_sem[T->id], &ts); // wait for S->time seconds
    int random_no = rand() % 100;
    if (random_no <= T->probability * 100)
    {
        //pthread_mutex_lock(&mutex);
        if (strcmp(T->team, "H") == 0)
        {
            pthread_mutex_lock(&mutex);
            count_H++;
            goal_H++;
            printf("%sTeam %s has scored their %dst goal\n", YEL, T->team, count_H);
            int released[1000] = {0};
            int release_count = 0;
            //  printf("%d\n",alloc_countA);
            for (int i = 0; i < actual_countA; i++)
            {
                // printf("-----%s\n",persons[allocA[i]].name);
                if (persons[actualA[i]].num_goals == goal_H)
                {
                    printf("%s%s is leaving due to bad performance of his team\n", CYN, persons[actualA[i]].name);
                    printf("%sPerson %s is leaving for dinner\n", CYN, persons[actualA[i]].name);
                    sem_post(&performance_sem[actualA[i]]);
                    released[release_count] = persons[actualA[i]].id;
                    release_count++;
                }
            }
            //printf("%d %d %d\n",release_count,countA,countN);
            int x = 0, count = 0, j = 0;
            for (int i = 0; i < countA; i++)
            {
                if (count == release_count)
                {

                    goto L;
                }
                while (statuswait_A[waitA[j]] == 0)
                {
                    j++;
                }
                printf("%s%s has got a seat in zone A\n", GRN, persons[waitA[j]].name);
                statuswait_A[waitA[j]] = 0;
                sem_post(&persons_sem[waitA[j]]);
                count++;
                j++;
            }
            j = 0;
            for (int i = countA; i < countN + countA; i++)
            {
                if (count == release_count)
                {
                    goto L;
                }
                while (statuswait_N[waitN[j]] == 0)
                {
                    j++;
                }
                printf("%s%s has got a seat in zone A\n", GRN, persons[waitN[j]].name);
                sem_post(&persons_sem[waitN[j]]);
                statuswait_N[waitN[j]] = 0;
                count++;
                j++;
            }
            int y = release_count - (countA + countN);
            wait_A = wait_A - y;
        L:;
            pthread_mutex_unlock(&mutex);
        }
        else
        {
            pthread_mutex_lock(&mutex);
            count_A++;
            goal_A++;
            printf("%sTeam %s has scored their %dst goal\n", YEL, T->team, count_A);
            pthread_mutex_unlock(&mutex);
            int released[1000] = {0};
            int release_count = 0;
            for (int i = 0; i < actual_countH; i++)
            {
                if (persons[actualH[i]].num_goals == goal_A)
                {
                    released[release_count] = persons[waitH[i]].id;
                    release_count++;
                    sem_post(&performance_sem[actualH[i]]);
                }
            }
            int x = 0, count = 0, j = 0;
            for (int i = 0; i < countH; i++)
            {
                if (count == release_count)
                {

                    goto L;
                }
                while (statuswait_H[waitH[j]] == 0)
                {
                    j++;
                }
                printf("%s%s has got a seat in zone H\n", GRN, persons[waitH[j]].name);
                statuswait_H[waitH[j]] = 0;
                sem_post(&persons_sem[waitH[j]]);
                count++;
                j++;
            }
            j = 0;
            for (int i = countH; i < countN + countH; i++)
            {
                if (count == release_count)
                {
                    goto L;
                }
                while (statuswait_N[waitN[j]] == 0)
                {
                    j++;
                }
                printf("%s%s has got a seat in zone A\n", GRN, persons[waitN[j]].name);
                sem_post(&persons_sem[waitN[j]]);
                statuswait_N[waitN[j]] = 0;
                count++;
                j++;
            }
            int y = release_count - (countH + countN);
            wait_A = wait_A - y;
        }
    }
    else
    {
        pthread_mutex_lock(&mutex);
        time = (clock() - starttime) / CLOCKS_PER_SEC;
        if (strcmp(T->team, "H") == 0)
        {
            count_H++;
            printf("%sTeam %s missed the chance to score their %dnd goal\n", YEL, T->team, count_H);

            //ticks = T->time;
        }
        else
        {
            count_A++;
            printf("%sTeam %s missed the chance to score their %dnd goal\n", YEL, T->team, count_A);
            //ticks = T->time;
        }
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void *Persons_func(void *args)
{
    Person *P = (Person *)args;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += P->time - 1;
    sem_timedwait(&persons_sem[P->id], &ts); // wait for S->time seconds
    printf("%s%s has reached the stadium\n", BLU, P->name);
    ticks[P->id] = P->time;
    int flag = 0;
    if (strcmp(P->zone, "H") == 0)
    {
        // while (1 != 0)
        // {
        pthread_mutex_lock(&mutex);
        if (wait_H <= capacity_H - 1)
        {
            //sem_wait(&persons_sem[P->id]);
            P->zone_time = ticks[P->id];
            //  pthread_mutex_lock(&mutex);
            printf("%s%s has got a seat in zone H\n", GRN, P->name);
            //waitH[countH] = P->id;
            //countH++;
            allocH[alloc_countH] = P->id;
            alloc_countH++;
            actualH[actual_countH] = P->id;
            actual_countH++;
            strcpy(P->occupied_zone, "H");
            wait_H++;
            pthread_mutex_unlock(&mutex);
            // break;
        }
        else if (wait_N <= capacity_N - 1)
        {
            seconds = time(NULL);
            //   pthread_mutex_lock(&mutex);
            printf("%s%s has got a seat in zone N\n", GRN, P->name);
            allocN[alloc_countN] = P->id;
            alloc_countN++;
            actualH[actual_countH] = P->id;
            actual_countH++;
            strcpy(P->occupied_zone, "N");
            wait_N++;
            pthread_mutex_unlock(&mutex);
            // break;
        }
        else
        {
            struct timespec waitfor_H;
            clock_gettime(CLOCK_REALTIME, &waitfor_H);
            waitfor_H.tv_sec += P->patience - 1;
            waitfor_H.tv_sec += 1;
            //  pthread_mutex_lock(&mutex);
            waitH[countH] = P->id;
            countH++;
            statuswait_H[P->id] = 1;
            pthread_mutex_unlock(&mutex);
            if (sem_timedwait(&persons_sem[P->id], &waitfor_H) == -1)
            {
                printf("%s%s could not get a seat\n", CYN, P->name);
                printf("%sPerson %s is leaving for dinner\n", CYN, P->name);
                return NULL;
            }
        }
        // }
    }
    if (strcmp(P->zone, "A") == 0)
    {
        // while (1 != 0)
        // {
        //pthread_mutex_lock(&mutex);
        pthread_mutex_lock(&mutex);
        if (wait_A <= capacity_A - 1)
        {
            //sem_wait(&persons_sem[P->id]);
            seconds = time(NULL);
            printf("%s%s has got a seat in zone A\n", GRN, P->name);
            allocA[alloc_countA] = P->id;
            alloc_countA++;
            actualA[actual_countA] = P->id;
            actual_countA++;
            strcpy(P->occupied_zone, "A");
            wait_A++;
            pthread_mutex_unlock(&mutex);
            // break;
        }
        else
        {
            struct timespec waitfor_A;
            clock_gettime(CLOCK_REALTIME, &waitfor_A);
            waitfor_A.tv_sec += P->patience - 1;
            waitfor_A.tv_sec += 1;
            //pthread_mutex_lock(&mutex);
            waitA[countA] = P->id;
            countA++;
            statuswait_A[P->id] = 1;
            pthread_mutex_unlock(&mutex);
            if (sem_timedwait(&persons_sem[P->id], &waitfor_A) == -1)
            {
                printf("%s%s could not get a seat\n", CYN, P->name);
                printf("%sPerson %s is leaving for dinner\n", CYN, P->name);
                return NULL;
            }
        }

        // }
    }
    if (strcmp(P->zone, "N") == 0)
    {
        // while (1 != 0)
        // {
        pthread_mutex_lock(&mutex);
        if (wait_N <= capacity_N - 1)
        {
            //sem_wait(&persons_sem[P->id]);
            seconds = time(NULL);
            //pthread_mutex_lock(&mutex);
            printf("%s%s has got a seat in zone N\n", GRN, P->name);
            allocN[alloc_countN] = P->id;
            alloc_countN++;
            actualN[actual_countN] = P->id;
            actual_countN++;
            strcpy(P->occupied_zone, "N");
            wait_N++;
            pthread_mutex_unlock(&mutex);
            //   break;
        }
        else if (wait_H <= capacity_H - 1)
        {
            seconds = time(NULL);
            // pthread_mutex_lock(&mutex);
            printf("%s%s has got a seat in zone H\n", GRN, P->name);
            allocH[alloc_countH] = P->id;
            alloc_countH++;
            actualN[actual_countN] = P->id;
            actual_countN++;
            strcpy(P->occupied_zone, "H");
            wait_H++;
            pthread_mutex_unlock(&mutex);
            //  break;
        }
        else if (wait_A <= capacity_A - 1)
        {
            seconds = time(NULL);
            // pthread_mutex_lock(&mutex);
            printf("%s%s has got a seat in zone A\n", GRN, P->name);
            allocA[alloc_countA] = P->id;
            alloc_countA++;
            actualN[actual_countN] = P->id;
            actual_countN++;
            strcpy(P->occupied_zone, "A");
            wait_A++;
            pthread_mutex_unlock(&mutex);
            // break;
        }
        else
        {
            struct timespec waitfor_N;
            clock_gettime(CLOCK_REALTIME, &waitfor_N);
            waitfor_N.tv_sec += P->patience - 1;
            waitfor_N.tv_sec += 1;
            waitN[countN] = P->id;
            countN++;
            statuswait_N[P->id] = 1;
            pthread_mutex_unlock(&mutex);
            if (sem_timedwait(&persons_sem[P->id], &waitfor_N) == -1)
            {
                printf("%s%s could not get a seat\n", CYN, P->name);
                printf("%sPerson %s is leaving for dinner\n", CYN, P->name);
                return NULL;
            }
        }
        // }
    }
    struct timespec watch_match;
    clock_gettime(CLOCK_REALTIME, &watch_match);
    watch_match.tv_sec += X - 1;
    watch_match.tv_sec += 1;
    if (sem_timedwait(&performance_sem[P->id], &watch_match) == -1)
    {
        printf("%s%s watched the match for %d seconds and is leaving\n", MAG, P->name, X);
        printf("%sPerson %s is leaving for dinner\n", MAG, P->name);
        if (strcmp(P->occupied_zone, "A") == 0)
        {
            wait_A--;
            for (int i = 0; i < countA; i++)
            {
                if (statuswait_A[P->id] == 1)
                {
                    printf("%s%s has got a seat in zone A\n", GRN, P->name);
                    statuswait_A[P->id] = 0;
                    sem_post(&persons_sem[waitA[i]]);
                    // wait_A--;
                    goto L;
                }
            }
            for (int i = 0; i < countN; i++)
            {
                if (statuswait_N[P->id] == 1)
                {
                    printf("%s%s has got a seat in zone N\n", GRN, P->name);
                    statuswait_N[P->id] = 0;
                    wait_N--;
                    sem_post(&persons_sem[waitN[i]]);
                    goto L;
                }
            }
        }
        if (strcmp(P->occupied_zone, "H") == 0)
        {
            wait_H--;
            for (int i = 0; i < countH; i++)
            {
                if (statuswait_H[P->id] == 1)
                {
                    printf("%s%s has got a seat in zone H\n", GRN, P->name);
                    statuswait_H[P->id] = 0;
                    sem_post(&persons_sem[waitH[i]]);
                    wait_H--;
                    goto L;
                }
            }
            for (int i = 0; i < countN; i++)
            {
                if (statuswait_N[P->id] == 1)
                {
                    printf("%s%s has got a seat in zone N\n", GRN, P->name);
                    statuswait_N[P->id] = 0;
                    wait_N--;
                    sem_post(&persons_sem[waitN[i]]);
                    goto L;
                }
            }
        }
        if (strcmp(P->occupied_zone, "N") == 0)
        {
            wait_N--;
            for (int i = 0; i < countN; i++)
            {
                if (statuswait_N[P->id] == 1)
                {
                    printf("%s%s has got a seat in zone N\n", GRN, P->name);
                    wait_N--;
                    statuswait_N[P->id] = 0;
                    sem_post(&persons_sem[waitN[i]]);
                    goto L;
                }
            }
            for (int i = 0; i < countH; i++)
            {
                if (statuswait_H[P->id] == 1)
                {
                    printf("%s%s has got a seat in zone H\n", GRN, P->name);
                    statuswait_H[P->id] = 0;
                    sem_post(&persons_sem[waitH[i]]);
                    wait_H--;
                    goto L;
                }
            }
        }
    L:;
    }
    /* else
    {
        printf("%s%s is leaving due to bad performance of his team\n", CYN, P->name);
    }*/
    // sem_wait(&persons_sem[P->id]);
    return NULL;
}
int main()
{
    init_semaphores();
    starttime = time(NULL);
    //printf("%ld\n",starttime);
    scanf("%d%d%d", &capacity_H, &capacity_A, &capacity_N);
    scanf("%d%d", &X, &num_groups);
    int count = 0;
    for (int i = 0; i < num_groups; i++)
    {
        ticks[i] = 0;
        scanf("%d", &groups[i]);
        for (int j = 0; j < groups[i]; j++)
        {
            scanf("%s%s%d%d%d", persons[count].name, persons[count].zone, &persons[count].time, &persons[count].patience, &persons[count].num_goals);
            persons[count].group = i;
            persons[count].id = count;
            count++;
        }
    }
    int chances;
    scanf("%d", &chances);
    for (int i = 0; i < chances; i++)
    {
        scanf("%s%d%lf", team_chances[i].team, &team_chances[i].time, &team_chances[i].probability);
        team_chances[i].id = i;
    }
    for (int i = 0; i < count; i++)
    {
        pthread_create(&(persons[i].Person_thread_id), NULL, Persons_func, &persons[i]);
    }
    for (int i = 0; i < chances; i++)
    {
        pthread_create(&(team_chances[i].goal_chance_id), NULL, Teams_func, &team_chances[i]);
    }
    for (int i = 0; i < count; i++)
    {
        pthread_join(persons[i].Person_thread_id, NULL);
    }
    for (int i = 0; i < chances; i++)
    {
        pthread_join(team_chances[i].goal_chance_id, NULL);
    }
}