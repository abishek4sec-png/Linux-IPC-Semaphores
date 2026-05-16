#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

#define NUM_LOOPS 20

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main() {
    int sem_set_id;
    int child_pid;
    int i;
    struct sembuf sem_op;
    union semun sem_val;

    sem_set_id = semget(IPC_PRIVATE, 1, 0600);
    if (sem_set_id == -1) {
        perror("semget");
        exit(1);
    }

    sem_val.val = 0;
    semctl(sem_set_id, 0, SETVAL, sem_val);

    child_pid = fork();

    if (child_pid == 0) {
        // consumer
        for (i = 0; i < NUM_LOOPS; i++) {
            sem_op.sem_num = 0;
            sem_op.sem_op = -1;
            sem_op.sem_flg = 0;
            semop(sem_set_id, &sem_op, 1);

            printf("consumer: %d\n", i);
            fflush(stdout);
        }
    } else {
        // producer
        for (i = 0; i < NUM_LOOPS; i++) {
            printf("producer: %d\n", i);
            fflush(stdout);

            sem_op.sem_num = 0;
            sem_op.sem_op = 1;
            sem_op.sem_flg = 0;
            semop(sem_set_id, &sem_op, 1);
        }

        wait(NULL);
        semctl(sem_set_id, 0, IPC_RMID, sem_val);
    }

    return 0;
}