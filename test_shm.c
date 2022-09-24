#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>


union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int del_sem(int sem_id) {
    union semun sem_union;
    if (semctl(sem_id,0,IPC_RMID,sem_union)==-1) {
        perror("Sem delete");
        exit(1);
    }
    return 0;
}

int sem_p(int sem_id) {
    struct sembuf sem_buf;
    sem_buf.sem_num=0;//信号量编号
    sem_buf.sem_op=-1;//P操作
    sem_buf.sem_flg=SEM_UNDO;//系统退出前未释放信号量，系统自动释放
    if (semop(sem_id,&sem_buf,1)==-1) {
        perror("Sem P operation");
        exit(1);
    }
    return 0;
}
// 对sem_id执行V操作
int sem_v(int sem_id) {
    struct sembuf sem_buf;
    sem_buf.sem_num=0;
    sem_buf.sem_op=1;//V操作
    sem_buf.sem_flg=SEM_UNDO;
    if (semop(sem_id,&sem_buf,1)==-1) {
        perror("Sem V operation");
        exit(1);
    }
    return 0;
}

int my_shmmem_open(char *file,int size,int open_flag){
    int shm_id;
    key_t key;
    key = ftok(file, 0x111);
    if(key == -1){
        printf("wrong key");
        return -1;
    }
    if(open_flag)
        shm_id = shmget(key, size, IPC_CREAT|IPC_EXCL|0600);
    else
        shm_id = shmget(key, 0, 0);
    if(shm_id == -1){
        printf("cant open");
        return -1;
    }
    return shm_id;
}

int my_shmmem_rm(int shm_id){
    shmctl(shm_id, IPC_RMID, NULL);
    return 0;
}

int main() {
    key_t sem_key=ftok("/home",0x22);
    printf("%d\n",sem_key);
    int sem_id=semget(sem_key,1,IPC_CREAT);
    printf("%d\n",sem_id);
    int shmid=my_shmmem_open("/",10*1024*1024,0);
    printf("%d\n",shmid);

    sem_p(sem_id);
    void *addr=shmat(shmid,NULL,0);
    perror("shmat");
    printf("%p",addr);
    printf("%s",(char*)addr);
    sem_v(sem_id);
    shmdt(addr);
    //my_shmmem_rm(shmid);
    //del_sem(sem_id);
    return 0;
}
