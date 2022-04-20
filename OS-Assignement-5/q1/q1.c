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

int starttime;
int track_of_filled_students[100];
int count = 0, waiting_count = 0;
sem_t student_sem[100];
sem_t course_sem[100];
sem_t tutorial_sem[100];
int p[100] = {0};
int student_alloc[100];
int x = 0;
int max_no1[100], max_no2[100], max_no3[100], current_preference[100];
int c_num, s_num, l_num;
pthread_mutex_t mutex, mut;
struct timespec ts;
int c_num, s_num, l_num;
typedef struct Student
{
    double calibre;
    int preference_1;
    int preference_2;
    int preference_3;
    int current_preference;
    int time;
    pthread_t student_thread_id;
    int id;
    int status;
    int exit;
} Student;

typedef struct pref_1
{
    float probability;
    int s_id;
} pref;

typedef struct Course
{
    char name[100];
    double interest;
    int max_slots;
    int no_of_labs;
    int lab[100];
    pthread_t course_thread_id;
    int id;
    int status;
    int random_slots;
    int current_allocatedTa;
    char current_lab[100];
    int count_pref1, count_pref2, count_pref3;
    int current_lab_id;
    int removed;
} Course;

typedef struct Labs
{
    char name[100];
    int no_of_Tas;
    int max_no;
    int id;
    int Ta_stat[100];
    int Ta_times[100];
    int status;
} Labs;

Student students[100];
Course courses[100];
Labs lab[100];
int w_list[100], wait_status_list[100];
void init_semaphores()
{
    for (int i = 0; i < 100; i++)
    {
        sem_init(&(student_sem[i]), 0, 0);
    }
    for (int i = 0; i < 100; i++)
    {
        sem_init(&(course_sem[i]), 0, 0);
    }
    for (int i = 0; i < 100; i++)
    {
        sem_init(&(tutorial_sem[i]), 0, 0);
    }
    //course_sem_pref2
    pthread_mutex_unlock(&mut);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mut, NULL);
}
void *courses_func(void *args)
{
    Course *C = (Course *)args;
    while (1 != 0)
    {
        if (x == s_num)
        {
            break;
        }
        pthread_mutex_lock(&mutex);
        if (C->removed == 1)
        {
            // printf("Hi\n");
            for (int i = 0; i < count; i++)
            {
                // sleep(1);
                // printf("Hi\n");
                if (students[w_list[i]].current_preference == C->id && wait_status_list[students[w_list[i]].id] != -1)
                {
                    if (students[w_list[i]].preference_1 == students[w_list[i]].current_preference)
                    {
                        printf("%sStudent %d has withdrawn from course %s\n", GRN, students[w_list[i]].id, C->name);
                        printf("%sStudent %d has changed current preference from %s (priority 1) to %s (priority 2)\n", GRN, students[w_list[i]].id, C->name, courses[students[w_list[i]].preference_2].name);
                        students[w_list[i]].current_preference = students[w_list[i]].preference_2;
                        current_preference[students[w_list[i]].id] = students[w_list[i]].preference_2;
                    }
                    else if (students[w_list[i]].preference_2 == students[w_list[i]].current_preference)
                    {
                        printf("%sStudent %d has withdrawn from course %s\n", GRN, students[w_list[i]].id, C->name);
                        printf("%sStudent %d has changed current preference from %s (priority 2) to %s (priority 3)\n", GRN, students[w_list[i]].id, C->name, courses[students[w_list[i]].preference_3].name);
                        students[w_list[i]].current_preference = students[w_list[i]].preference_3;
                        current_preference[students[w_list[i]].id] = students[w_list[i]].preference_3;
                    }
                    else if (students[w_list[i]].preference_3 == students[w_list[i]].current_preference)
                    {
                        printf("%sStudent %d couldn’t get any of his preferred courses\n", GRN, students[w_list[i]].id);

                        wait_status_list[students[w_list[i]].id] = -1;
                        students[students[w_list[i]].id].status = 1;
                        x++;
                        students[students[w_list[i]].id].exit = 1;
                    }
                    p[students[w_list[i]].id] = 1;
                    sem_post(&student_sem[students[w_list[i]].id]);
                }
            }
            pthread_mutex_unlock(&mutex);
            continue;
        }
        int flag = 0;
        /*  for (int i = 0; i < C->no_of_labs; i++)
        {
            if(i>0&&lab[C->lab[i]].status == 0)
            {
                printf("%sLab %s no longer has students available for TA ship\n",WHT,lab[C->lab[i]].name);
                lab[C->lab[i]].status=1;
            }
            if(lab[C->lab[i]].status == 0)
            {
            for (int j = 0; j < lab[C->lab[i]].no_of_Tas; j++)
            {
                if (lab[C->lab[i]].max_no > lab[C->lab[i]].Ta_times[j])
                {
                    printf("%sTA %d from lab %s has been allocated to course %s for his %dth TA ship\n", BLU, j, lab[C->lab[i]].name, C->name, (lab[C->lab[i]].Ta_times[j]) + 1);
                    C->current_allocatedTa = j;
                    strcpy(C->current_lab, lab[C->lab[i]].name);
                    C->current_lab_id = C->lab[i];
                    lab[C->lab[i]].Ta_times[j]++; //
                    flag = 1;
                    goto L;
                }
            }
            }
        }*/
        for (int i = 0; i < C->no_of_labs; i++)
        {
            for (int j = 0; j < lab[C->lab[i]].no_of_Tas; j++)
            {
                if (lab[C->lab[i]].max_no > lab[C->lab[i]].Ta_times[j])
                {
                    printf("%sTA %d from lab %s has been allocated to course %s for his %dth TA ship\n", BLU, j, lab[C->lab[i]].name, C->name, (lab[C->lab[i]].Ta_times[j]) + 1);
                    C->current_allocatedTa = j;
                    strcpy(C->current_lab, lab[C->lab[i]].name);
                    C->current_lab_id = C->lab[i];
                    lab[C->lab[i]].Ta_times[j]++; //
                    flag = 1;
                    goto L;
                }
            }
            if (lab[C->lab[i]].status == 0)
            {
                printf("%sLab %s no longer has students available for TA ship\n", WHT, lab[C->lab[i]].name);
                lab[C->lab[i]].status = 1;
            }
        }

    L:;
        if (flag == 0)
        {
            printf("%sCourse %s doesn’t have any TA’s eligible and is removed from course offerings\n", YEL, C->name);
            C->removed = 1;
            //printf("hi\n");
            for (int i = 0; i < count; i++)
            {
                //printf("Hi\n");
                if (students[w_list[i]].current_preference == C->id && wait_status_list[students[w_list[i]].id] != -1)
                {
                    if (students[w_list[i]].preference_1 == students[w_list[i]].current_preference)
                    {
                        printf("%sStudent %d has withdrawn from course %s\n", GRN, students[w_list[i]].id, C->name);
                        printf("%sStudent %d has changed current preference from %s (priority 1) to %s (priority 2)\n", GRN, students[w_list[i]].id, C->name, courses[students[w_list[i]].preference_2].name);
                        students[w_list[i]].current_preference = students[w_list[i]].preference_2;
                        current_preference[students[w_list[i]].id] = students[w_list[i]].preference_2;
                    }
                    else if (students[w_list[i]].preference_2 == students[w_list[i]].current_preference)
                    {
                        printf("%sStudent %d has withdrawn from course %s\n", GRN, students[w_list[i]].id, C->name);
                        printf("%sStudent %d has changed current preference from %s (priority 2) to %s (priority 3)\n", GRN, students[w_list[i]].id, C->name, courses[students[w_list[i]].preference_3].name);
                        students[w_list[i]].current_preference = students[w_list[i]].preference_3;
                        current_preference[students[w_list[i]].id] = students[w_list[i]].preference_3;
                    }
                    else if (students[w_list[i]].preference_3 == students[w_list[i]].current_preference)
                    {
                        printf("%sStudent %d couldn’t get any of his preferred courses\n", GRN, students[w_list[i]].id);

                        wait_status_list[students[w_list[i]].id] = -1;
                        students[students[w_list[i]].id].status = 1;
                        x++;
                        students[students[w_list[i]].id].exit = 1;
                    }
                    p[students[w_list[i]].id] = 1;
                    sem_post(&student_sem[students[w_list[i]].id]);
                }
            }
            pthread_mutex_unlock(&mutex);
            continue;
        }
        pthread_mutex_unlock(&mutex);
        int random_no = (rand() % C->max_slots) + 1;
        printf("%sCourse %s has been allocated %d seats\n", MAG, C->name, random_no);
        int alloc_count = 0;
        int studentlist[100] = {0};
        // printf("%d %d\n",random_no,count);
        for (int i = 0; i < count; i++)
        {
            if (alloc_count >= random_no)
            {
                break;
            }
            if (students[w_list[i]].current_preference == C->id && wait_status_list[students[w_list[i]].id] != -1)
            {
                studentlist[alloc_count] = students[w_list[i]].id;
                sem_post(&student_sem[students[w_list[i]].id]);
                alloc_count++;
            }
        }
        if (alloc_count == 0)
        {
            printf("%sTutorial can not be started with zero seats allocated for %s\n", YEL, C->name);
            lab[C->current_lab_id].Ta_times[C->current_allocatedTa]--;
            sleep(2);
            continue;
        }
        while (1 != 0)
        {
            if (alloc_count == student_alloc[C->id])
            {
                printf("%sTutorial has started for Course %s with %d seats filled out of %d\n", YEL, C->name, alloc_count, random_no);
                break;
            }
        }
        sleep(2);
        printf("%sTA %d from lab %s has completed the tutorial for course %s\n", YEL, C->current_allocatedTa, C->current_lab, C->name);
        C->random_slots = alloc_count;
        student_alloc[C->id] = 0;
        for (int i = 0; i < C->random_slots; i++)
        {
            int rand_number = rand() % 100 + 1;
            if (rand_number <= (C->interest * students[studentlist[i]].calibre) * 100)
            {
                printf("%sStudent %d has selected the course %s permanently\n", GRN, studentlist[i], C->name);
                wait_status_list[studentlist[i]] = -1;
                students[studentlist[i]].status = 1;
                sem_post(&student_sem[studentlist[i]]);
                x++;
            }
            else
            {
                if (students[studentlist[i]].preference_1 == students[studentlist[i]].current_preference)
                {
                    printf("%sStudent %d has withdrawn from course %s\n", GRN, studentlist[i], C->name);
                    printf("%sStudent %d has changed current preference from %s (priority 1) to %s (priority 2)\n", GRN, studentlist[i], C->name, courses[students[studentlist[i]].preference_2].name);
                    students[studentlist[i]].current_preference = students[studentlist[i]].preference_2;
                    current_preference[studentlist[i]] = students[studentlist[i]].preference_2;
                }
                else if (students[studentlist[i]].preference_2 == students[studentlist[i]].current_preference)
                {
                    printf("%sStudent %d has withdrawn from course %s\n", GRN, studentlist[i], C->name);
                    printf("%sStudent %d has changed current preference from %s (priority 2) to %s (priority 3)\n", GRN, studentlist[i], C->name, courses[students[studentlist[i]].preference_3].name);
                    students[studentlist[i]].current_preference = students[studentlist[i]].preference_3;
                    current_preference[studentlist[i]] = students[studentlist[i]].preference_3;
                }
                else if (students[studentlist[i]].preference_3 == students[studentlist[i]].current_preference)
                {
                    printf("%sStudent %d couldn’t get any of his preferred courses\n", GRN, studentlist[i]);
                    wait_status_list[studentlist[i]] = -1;
                    students[studentlist[i]].status = 1;
                    x++;
                    students[studentlist[i]].exit = 1;
                }
                sem_post(&student_sem[studentlist[i]]);
            }
        }
    }
    return NULL;
}
void *students_func(void *args)
{
    Student *S = (Student *)args;
    int time;
    // pthread_mutex_lock(&mutex);
    time = (clock() - starttime) / CLOCKS_PER_SEC;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += S->time - 1;
    sem_timedwait(&student_sem[S->id], &ts); // wait for S->time seconds
    printf("%sStudent %d has filled in preferences for course registration\n", RED, S->id);
    current_preference[S->id] = S->preference_1;
    S->current_preference = S->preference_1;
    w_list[count] = S->id;
    count++;
    wait_status_list[S->id] = 1;
    //  pthread_mutex_unlock(&mutex);
    // printf("%d\n",count);
    sem_wait(&student_sem[S->id]);
    if (p[S->id] != 0)
    {
        // printf("Hello I am here %d\n",S->id);
        p[S->id] = 0;
        goto L2;
    }
    printf("%sStudent %d has been allocated a seat in course %s\n", CYN, S->id, courses[current_preference[S->id]].name);
    student_alloc[courses[current_preference[S->id]].id]++;
    sem_wait(&student_sem[S->id]);
// printf("%d\n",S->status);
L2:;
    if (S->status == 0)
    {
        sem_wait(&student_sem[S->id]);
        if (p[S->id] != 0)
        {
            p[S->id] = 0;
            goto L3;
        }
        printf("%sStudent %d has been allocated a seat in course %s\n", CYN, S->id, courses[current_preference[S->id]].name);
        student_alloc[courses[current_preference[S->id]].id]++;
        sem_wait(&student_sem[S->id]);
    L3:;
        if (S->status == 0)
        {
            if (p[S->id] != 0)
            {
                p[S->id] = 0;
                return NULL;
            }
            sem_wait(&student_sem[S->id]);
            printf("%sStudent %d has been allocated a seat in course %s\n", CYN, S->id, courses[current_preference[S->id]].name);
            student_alloc[courses[current_preference[S->id]].id]++;
        }
    }
    return NULL;
}

int main()
{
    init_semaphores();
    for (int i = 0; i < 100; i++)
    {
        w_list[i] = -1;
        wait_status_list[i] = -1;
    }
    starttime = clock();
    scanf("%d%d%d", &s_num, &l_num, &c_num);
    for (int i = 0; i < c_num; i++)
    {
        student_alloc[courses[i].id] = 0;
        courses[i].removed = 0;
        courses[i].count_pref1 = 0;
        courses[i].count_pref2 = 0;
        courses[i].count_pref3 = 0;
        courses[i].random_slots = 0;
        courses[i].id = i;
        courses[i].status = 0;
        scanf("%s%lf%d%d", courses[i].name, &courses[i].interest, &courses[i].max_slots, &courses[i].no_of_labs);
        int num = courses[i].no_of_labs;
        for (int j = 0; j < num; j++)
        {
            scanf("%d", &courses[i].lab[j]);
        }
    }
    for (int i = 0; i < s_num; i++)
    {
        students[i].exit = 0;
        students[i].status = 0;
        // printf("%d %d %d\n",students[i].preference_1,students[i].preference_2,students[i].preference_3);
        students[i].id = i;
        scanf("%lf%d%d%d%d", &students[i].calibre, &students[i].preference_1, &students[i].preference_2, &students[i].preference_3, &students[i].time);
    }
    for (int i = 0; i < l_num; i++)
    {
        lab[i].id = 0;
        lab[i].id = i;
        for (int j = 0; j < 100; j++)
        {
            lab[i].Ta_times[j] = 0;
        }
        scanf("%s%d%d", lab[i].name, &lab[i].no_of_Tas, &lab[i].max_no);
        for (int j = 0; j < lab[i].no_of_Tas; j++)
        {
            lab[i].Ta_stat[j] = 0;
        }
    }
    for (int i = 0; i < s_num; i++)
    {
        pthread_create(&(students[i].student_thread_id), NULL, students_func, &students[i]);
    }
    for (int i = 0; i < c_num; i++)
    {
        pthread_create(&(courses[i].course_thread_id), NULL, courses_func, &courses[i]);
    }

    for (int i = 0; i < s_num; i++)
    {
        pthread_join(students[i].student_thread_id, NULL);
    }
    for (int i = 0; i < c_num; i++)
    {
        pthread_join(courses[i].course_thread_id, NULL);
    }
}