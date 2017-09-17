#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include<stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <semaphore.h>


#define MAXFCTSIZE 128
#define MAXPROCESSES 16384

#define MAXSLAVE 8192
#define MAXTASKTYPE 8192
#define QUEUESIZE 8192

struct tasktype
{
  char name[200];
  int time;  
};

struct slave
{
   char name[200];
   //int instances;
   int qno;
   bool flag;
   int head;    //index of tasktype
   int count;
   int time_stamp;
   bool hasallottedjob;     
};


struct process
{
   int pno;
   char pname[200];
   char slavename[200];
   char task_type_name[200];
   int time; 
   int processflag;
   char scheduled;
 };


struct taskdone
{
   char taskname[200];
   int processcounter;   
};


struct forkchildtable
{
  int pid;
  int totalnooftasks;
  char taskname[200];
  struct taskdone t[1024];
  int count;
};

struct Output
{
   char pname[200];
   char slavename[200];
   char task_type_name[200];
   int pid;
   int waiting;
   char finishstatus[10];      
};


struct Queue
{
    char slavename[200];
    //pid_t pid;
    int rear;
    int front;
    int pno[QUEUESIZE];
    int prindex[QUEUESIZE];
    struct process p[QUEUESIZE];
};


struct scheduleprocesses
{
   int startindex;
   int endindex;
   int currentindex;
};



void main(int argc , char *argv[])
{
   /* 
    int argc = 4;
    char *argv[] = {"./Assignment5", "Sample2-slave.info", "Sample2-job.info","110"};
    */
    if(argc != 4)
    {
       fprintf(stderr , "Invalid no of arguments \n");
       exit(0);
    }
    int m = atoi(argv[3]);
    if(m <= 0)
    {
      fprintf(stderr , "Invalid value of m\n");
      exit(0);
    }
    int shmidtscounter, shmid0, shmid1  ,shmid3 ,  shmid5, shmid6 , shmid7 , shmid8;
    int shmidoutput;
    struct forkchildtable *fct;
    struct taskdone *td;
    struct process *pr;
    struct ptask *ptsk;
    struct slave *slv;
    struct tasktype *tt;
    struct Queue *q;
    struct scheduleprocesses *sp;
    int *tscounter;
       
    int noOfQueues = 0 ;
    
    sem_t *mutex;
    shmid0 = shmget(IPC_PRIVATE, sizeof(sem_t), IPC_CREAT | 0660);
    if (shmid0 < 0) exit(1);
    mutex = (sem_t *)shmat(shmid0, NULL, 0);
    if (mutex == (void *) (-1)) exit(1);
    //printf("shmid0 is %d \n", shmid0); 
    sem_init(mutex, 1, 1);
    
    
    shmid1 = shmget(IPC_PRIVATE, 100*sizeof(struct forkchildtable), IPC_CREAT | 0660);
    if (shmid1 < 0) exit(1);
    fct = (struct forkchildtable *)shmat(shmid1, NULL, 0);
    if (fct == (void *) (-1)) exit(1);
    
   
      
   
    
    shmidtscounter = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0660);
    if (shmidtscounter < 0) exit(1);
    tscounter = (int *)shmat(shmidtscounter, NULL, 0);
    if (tscounter == (void *) (-1)) exit(1);
    //printf("shmidtscounter is %d \n", shmidtscounter); 
    *tscounter = 0;
    
    int cntpr = 0, cntslv =0 , cntfct = 0, cnttt = 0 , cntsp = 0;
      
   
    //printf("shmid1 is %d \n", shmid1);
    

    shmid3 = shmget(IPC_PRIVATE, MAXPROCESSES*sizeof(struct process), IPC_CREAT | 0660);
    if (shmid3 < 0) exit(1);
    pr = (struct process *)shmat(shmid3, NULL, 0);
    if (pr == (void *) (-1)) exit(1);
    //printf("shmid3 is %d\n", shmid3);
    
    
    
    shmid5 = shmget(IPC_PRIVATE, MAXSLAVE*sizeof(struct slave), IPC_CREAT | 0660);
    if (shmid5 < 0) exit(1);
    slv = (struct slave *)shmat(shmid5, NULL, 0);
    if (slv == (void *) (-1)) exit(1);
    //printf("shmid is %d\n", shmid5);

    shmid6 = shmget(IPC_PRIVATE, MAXTASKTYPE*sizeof(struct tasktype), IPC_CREAT | 0660);
    if (shmid6 < 0) exit(1);
    tt = (struct tasktype *)shmat(shmid6, NULL, 0);
    if (tt == (void *) (-1)) exit(1);
    //printf("shmid6 is %d\n", shmid6);
    
    shmid8 = shmget(IPC_PRIVATE, m*sizeof(struct scheduleprocesses), IPC_CREAT | 0660);
    if (shmid8 < 0) exit(1);
    sp = (struct scheduleprocesses *)shmat(shmid8, NULL, 0);
    if (sp == (void *) (-1)) exit(1);
    //printf("shmid8 is %d\n", shmid8);
    
    FILE *fpout;
    struct Output *output;
    shmidoutput = shmget(IPC_PRIVATE, 100000 * sizeof(struct Output), IPC_CREAT | 0660);
    if (shmidoutput < 0) exit(1);
    output = (struct Output *)shmat(shmidoutput, NULL, 0);
    if (output == (void *) (-1)) exit(1);
    //printf("shmidoutput is %d\n", shmidoutput);
    
    int *cntoutput;
    int shmidcnt;
    shmidcnt = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0660);
    if (shmidcnt < 0) exit(1);
    cntoutput = (int *)shmat(shmidcnt, NULL, 0);
    if (cntoutput == (void *) (-1)) exit(1);
   // printf("shmidcnt is %d\n", shmidcnt);
    
    fpout = fopen("CommandOutput.txt" , "w");   
    
    
    
    int i,j;
    //printf("Test1");
 
    //printf("Test2");    
    for(i = 0 ; i < MAXSLAVE ; i++)
        slv[i].count = 0;
    //printf("Test3");
    for(i = 0 ; i < 100 ; i++)
         fct[i].count = 0;    
    //printf("Test4");

    
    FILE *fp; 
    fp = fopen(argv[1] , "r");   //slave file
    if(fp == NULL)
    {
       printf("Error in opening %s file\n", argv[1]);
       exit(0);
    }
    
    
    //char buffer[100];
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *t1 , *t2;
    t1 = (char *) malloc(200 * sizeof(char));
    t2 = (char *) malloc(200 * sizeof(char));
    //create queues 
    int countQueues = 0;
    
    while((read = getline(&line, &len , fp)) != -1)
    {
      // printf("%s\n read = %d",line, (int)read);
      // printf("CountQueues = %d \n",countQueues);
      
       countQueues++;
    }   
    
    fclose(fp);
    
 
    
    shmid7 = shmget(IPC_PRIVATE, countQueues*sizeof(struct Queue), IPC_CREAT | 0660);
    if (shmid7 < 0) exit(1);
    q = (struct Queue *)shmat(shmid7, NULL, 0);
    if (q == (void *) (-1)) exit(1);
    //printf("shmid7 is %d\n", shmid7);

        
    
    fp = fopen(argv[1] , "r");
         
    while((read = getline(&line , &len , fp)) != -1)
    {
        char *text;
        text = (char *) malloc (strlen(line) * sizeof(char));
        strcpy(text , line);
        char *token1;
        token1 = strtok(text , " ");
        bool flagcheck = true;
        for(i = 0 ; i < noOfQueues ; i++)
        {
           if(strcmp(q[i].slavename , token1) == 0)
           {
              flagcheck = false;
              break;
           } 
        }
        
        
        if(token1 != NULL && flagcheck)
        {          
           strcpy(q[noOfQueues].slavename , token1);
           q[noOfQueues].front = q[noOfQueues].rear = 0;          
           noOfQueues++;  
        }
    }
    
    //printf(" No of queues = %d \n",noOfQueues);
    fclose(fp);
    fp = fopen(argv[1] , "r");
    
    
    
   //printf("Flag delete is %s " , flagDelete ? "true" : "false");
    while ((read = getline(&line, &len, fp)) != -1) 
    {
       char *text;
       text = (char *) malloc(strlen(line) * sizeof(char));
       strcpy(text , line);
       //printf("%s\n",text);
       char *token1 , *token2;  
       token1 = strtok(text , " ");
       token2 = strtok(NULL , " ");
       strcpy(t1 , token1);
       strcpy(t2, token2);
       
       //printf(" Test1 %s %s \n ", t1 , t2);
       if(token1 == NULL || token2 == NULL)
          break;
       int instances = atoi(t2);
       //printf(" %s Instances %d \n" ,t1 , instances);
       if(instances <= 0)
       {
         fprintf(stderr , "Error in no of instances of slave\n");
         exit(0);
       } 
       //printf(" t1 %s \n" , t1);  
       //printf(" Test2 ");
       strcpy(slv[cntslv].name , t1);
       slv[cntslv].flag = false;
       //printf("Test3");
       slv[cntslv].head = cnttt;
       slv[cntslv].time_stamp = *tscounter;
       slv[cntslv].hasallottedjob = false;
       int queueno;
       for(queueno = 0 ;  queueno < noOfQueues; queueno++)
       {
          if(strcmp(slv[cntslv].name , q[queueno].slavename) == 0)
          {
             //printf("slave %s is allocated queue %d\n " , slv[cntslv].name , queueno);
             //getchar();
             slv[cntslv].qno = queueno;
             break;
          }   
       }   
       
       
       int n = 0 ;
       while(1)
       {
          token1 = strtok(NULL , " ");
          token2 = strtok(NULL , " ");
          if(token1 == NULL || token2 == NULL)
             break;
          strcpy(t1 , token1);
          strcpy(t2, token2);
          printf("%s %s \n",t1, t2);
          int tm = atoi(t2);
          if(tm <= 0)
          {
            fprintf(stderr , "Error in time value\n");
            exit(0);
          }
          strcpy(tt[cnttt].name, t1);
          tt[cnttt].time = tm;
          slv[cntslv].count++;
          cnttt++;
          n++;          
       }
       //printf("Value of n %d \n",n);
       cntslv++;
       //printf("%d cntslv \n ", cntslv);
       for(i = 1; i < instances ; i++)
       {
           // printf("Testing123\n");       
            strcpy(slv[cntslv].name , slv[cntslv - 1].name);
            slv[cntslv].flag = false;
            slv[cntslv].head = cnttt;
            slv[cntslv].qno = queueno;
            slv[cntslv].count = 0;
            slv[cntslv].time_stamp = *tscounter;
            slv[cntslv].hasallottedjob = false;
            for(j = 0 ; j < n ; j++)
            {
              //  printf("Testing234\n");
                strcpy(tt[cnttt].name , tt[cnttt-n].name);
                tt[cnttt].time = tt[cnttt-n].time;
                slv[cntslv].count++;
                cnttt++;
            }
            cntslv++;
       }      
          
    }
    
    
     
   
    /* To display slaves */
    /*
    for(i = 0 ; i < cntslv; i++)
    {
       printf(" %s  %d %d \n" , slv[i].name, slv[i].head,  slv[i].count);
       for(j = slv[i].head; j<slv[i].head + slv[i].count; j++)
           printf("\t %s %d \n " , tt[j].name , tt[j].time);
    }
    
    */  
    char *prname = (char *) malloc(200 * sizeof(char));
    int prno = 0;   
   //printf("Flag delete is %s " , flagDelete ? "true" : "false");
    do
    {
      fp = fopen(argv[2] , "r");   //job file
      if(fp == NULL)
      {
        fprintf(stderr , "Error in opening %s file\n",argv[2]);
        exit(0);
      }
     
      while ((read = getline(&line, &len, fp)) != -1 && prno < m) 
      {
        //printf("cntpr is %d \n " , cntpr);
        sp[cntsp].startindex = cntpr;  
        char *text;
        text = (char *) malloc(strlen(line) * sizeof(char));
        strcpy(text , line);
        //printf("%s\n",text);
        char *token1 , *token2;  
        token1 = strtok(text , " ");
       
        strcpy(t1 , token1);
        strcpy(prname, t1);
       //printf(" Test1 %s %s \n ", t1 , t2);
        if(token1 == NULL)
          break;
       //printf(" t1 %s \n" , t1);  
       //printf(" Test2 ");
        while(1)
        {
          token1 = strtok(NULL , " ");
          if(token1 == NULL)
             break;
          strcpy(pr[cntpr].pname , prname);
      
          //printf("Test3");
          pr[cntpr].pno = prno;
          pr[cntpr].scheduled = 'N';
          strcpy(t1 , token1);
          i = 0;
          while(t1[i] != '\0' && t1[i] != '\n')
               i++;
          t1[i] = '\0';
          //printf("t1 is = %s \n" , t1);
          i = 0;
          while(t1[i] != ':' && i < strlen(t1))
          {
             //printf("t1[%d] = %c \n", i, t1[i]);
             pr[cntpr].slavename[i] = t1[i];
             
             //printf("pr[cntpr].slavename[%d] = %c \n",i, pr[cntpr].slavename[i]);
             i++;
          }
          pr[cntpr].slavename[i] = '\0';
         // printf("Slave name %s \n " , pr[cntpr].slavename);
          j = 0; i++;
          while(i < strlen(t1))
             pr[cntpr].task_type_name[j++] = t1[i++];
          pr[cntpr].task_type_name[j] = '\0';
          //printf("Task type name %s \n",pr[cntpr].task_type_name);
          pr[cntpr].processflag = 0;
          bool flg = false;
          for(i = 0 ; i < cntslv && !flg ; i++)
          {
              //printf("Testing  :  %s %s \n",slv[i].name, pr[cntpr].slavename);
              if(strcmp(slv[i].name , pr[cntpr].slavename) == 0)
              {
                  for(j = slv[i].head ; j < slv[i].head + slv[i].count ; j++)
                  {
                     //printf(" j = %d , %s , %s , %d\n", j , tt[j].name, pr[cntpr].task_type_name, tt[j].time);
                     if(strcmp(tt[j].name, pr[cntpr].task_type_name) == 0)
                     {
                        pr[cntpr].time = tt[j].time;
                        //printf(" pr[%d].time is assinged %d time \n",cntpr, tt[j].time);
                        flg = true;
                        break;
                     }
                  }
              }
          }
          cntpr++;
          
       }
       sp[cntsp].endindex = cntpr - 1;
       sp[cntsp].currentindex = 0;
       //printf("%d %d %d \n " , sp[cntsp].startindex , sp[cntsp].endindex, sp[cntsp].currentindex);
       cntsp++;
       prno++;
       //free(text);
     }
     fclose(fp); 
   }while(prno < m);  
    
    /* To display all processes */
    /*
    for(i = 0 ; i < cntpr ; i++)
    {
       printf(" %d %s %s %s %d  %d\n", pr[i].pno , pr[i].pname , pr[i].slavename, pr[i].task_type_name,pr[i].time , pr[i].processflag);
    }
    */
    
    //printf("Schedulingprocess array is :\n");
    /*
    for(i = 0 ; i < m ; i++)
    {
       printf(" %d %d %d \n" , sp[i].startindex , sp[i].endindex, sp[i].currentindex);
    }
    */
    
    
    for(i = 0 ; i<cntslv; i++)
    {
        //q[i].front = 0; q[i].rear = 0;
        //strcpy(q[i].slavename , slv[i].name);
        fct[i].totalnooftasks = 0;
        fct[i].count=0;
        strcpy(fct[i].taskname , slv[i].name);
        int k;
        for(j = slv[i].head , k = 0 ; j < slv[i].head + slv[i].count; j++,k++)
         {
            strcpy(fct[i].t[k].taskname , tt[j].name);
            fct[i].t[k].processcounter = 0;
            fct[i].count++;
         }
    }
    
    /*
    int qno;    
    for(i = 0 ; i < m ; i++)
    {
       int k = sp[i].startindex + sp[i].currentindex;
       if(pr[k].processflag  == 0)
       {
          for(qno = 0 ; qno < noOfQueues ; qno++)
          {
            if(strcmp(pr[k].slavename , q[qno].slavename) == 0)
               break;
          }
       
          q[qno].prindex[q[qno].rear] = k;
          q[qno].pno[q[qno].rear]  = i;
          q[qno].p[q[qno].rear++] = pr[k];          
          pr[k].scheduled = 'Y';
       }
    }
    
    */
    //To display Queue
    /*
    for(i = 0 ; i < noOfQueues; i++)
    {
        printf("Queue no %d %s %d %d\n" , i,q[i].slavename , q[i].front , q[i].rear);
        for(j = q[i].front ; j < q[i].rear ; j++)
        {
           printf("index = %d  prno = %d   %d %s %s %s %d %d %c\n " , q[i].prindex[j], q[i].pno[j], q[i].p[j].pno , q[i].p[j].pname , q[i].p[j].slavename , q[i].p[j].task_type_name, q[i].p[j].time , q[i].p[j].processflag , q[i].p[j].scheduled);           
        }
    }
    */
    //getchar();
    
    pid_t *pid = (pid_t *) malloc(cntslv * sizeof(pid_t));
    
    
    for(i = 0 ; i < cntslv ; i++)
    {
       pid[i] = fork();
       
       if(pid[i] == 0)
       {  //child
          //sem_wait(mutex); 
           
          int qno = slv[i].qno;
          //printf(" No of slaves = %d No of queues %d slv[i].name = %s\n" ,cntslv,  noOfQueues, slv[i].name);
          
                    
          //getchar();
          
          
          //printf("%s %d \n",q[qno].slavename, (int)getpid());
          fct[i].pid = (int)getpid();
          //sem_post(mutex);
          
          while(1)
          {
             //printf("Check %s %d \n " , q[i].slavename, i);
             //getchar();
             int checkslave;
             bool checkslaveflag = false;
             for(checkslave = 0 ; checkslave < cntslv  ; checkslave++)
             {
                 if(checkslave == i)
                    continue;
                 sem_wait(mutex);   
                 if(slv[checkslave].qno == slv[i].qno && slv[checkslave].hasallottedjob == false && slv[checkslave].time_stamp < slv[i].time_stamp)
                 {
                       checkslaveflag = true;
                       sem_post(mutex);
                       break;
                 }
                 sem_post(mutex);       
             }
             if(checkslaveflag)
             {
                //sleep(0.05);
                continue;
             }
             int r ,f;
             sem_wait(mutex);
             r = q[qno].rear;
             f = q[qno].front; 
                            
             //printf("pid = %d  qno = %d front = %d rear = %d\n ", (int)getpid(), qno, f , r);
             //getchar();
             //printf("\n%d  Child entered\n",getpid());
               
             
             
             if(r > f)
             {
                //printf(" Child %d is running for process %s\n" , i , q[i].slavename);
                //printf("pid = %d r > f is true \n" , getpid());
                //getchar();
                slv[i].hasallottedjob = true;
                
                q[qno].front++;
                sem_post(mutex);
                
                sleep(q[qno].p[q[qno].front].time / 1000);
                 
                sem_wait(mutex);
                slv[i].hasallottedjob = false;
                (*tscounter)++;
                slv[i].time_stamp = *tscounter;
                q[qno].p[f].processflag = 1;
                pr[q[qno].prindex[f]].processflag = 1;
                //printf("%d Current index of %d " , (int)getpid() , sp[q[qno].pno[q[qno].front]].currentindex); 
                sp[q[qno].pno[f]].currentindex++;
                //printf("%d sp s index is %d is set to %d " ,(int)getpid(), q[qno].pno[q[qno].front] , sp[q[qno].pno[q[qno].front]].currentindex);
                //getchar();
                int indx = q[qno].prindex[f];
                int prno = q[qno].pno[f];
               // printf("%d prno is %d indx is %d  " ,(int)getpid(),indx,  prno);
                //printf(" sp[prno].endindex = %d sp[prno].currentindex = %d \n",sp[prno].endindex , sp[prno].currentindex);
                //getchar();
                int waiting = sp[prno].endindex - sp[prno].startindex + 1 - sp[prno].currentindex;
                
                printf("%s %s:%s %d %d ",pr[indx].pname, pr[indx].slavename, pr[indx].task_type_name, (int)getpid(), waiting );
                
                strcpy(output[*cntoutput].pname , pr[indx].pname);
                strcpy(output[*cntoutput].slavename , pr[indx].slavename);
                strcpy(output[*cntoutput].task_type_name , pr[indx].task_type_name);
                output[*cntoutput].pid = (int)getpid();
                output[*cntoutput].waiting = waiting;
                
                if(waiting == 0)
                {
                   printf(" finished\n");
                   strcpy(output[(*cntoutput)++].finishstatus, "finished");
                }
                else
                {
                   printf(" waiting\n");
                   strcpy(output[(*cntoutput)++].finishstatus, "waiting");
                }
                
                fct[i].totalnooftasks++;
                int itr; bool checkflag = false;
                for(itr = 0 ; itr < fct[i].count; itr++)
                {
                     if(strcmp(fct[i].t[itr].taskname , pr[indx].task_type_name) == 0)
                     {
                         checkflag = true;
                         fct[i].t[itr].processcounter++;
                         break;
                     }    
                }   
                if(!checkflag)
                {
                   fprintf(stderr , "%s Error in name of task\n", pr[indx].task_type_name);
                }                    
                sem_post(mutex);    
                  
                //printf("front incremented\n");                              
                //sem_post(mutex);
             }
             else
                sem_post(mutex);
                        
          }
          _exit(0);
       }
        
       else
       { // parent
          fprintf(fpout,"%s %d \n",slv[i].name, pid[i]);
          printf("%s %d \n",slv[i].name , pid[i]);        
          //printf("Parent running i = %d \n",i);
          //getchar();
          continue;
       }
    }
    
   // printf("Out side for loop parent running \n");
    //getchar();
    int prcontinueflag = 0;
    
    do
    {   
        prcontinueflag = 0;
        sem_wait(mutex);
        
      
        for(i = 0 ; i < cntpr ; i++)
        {
           if(pr[i].processflag == 0)
              prcontinueflag = 1;
        }
        
        if(prcontinueflag == 0)
        {
          
           break;
        }
    //    printf("Parent prcontflag is %d \n" ,prcontinueflag);
     //   getchar();
          
        for(i = 0 ; i < m ; i++)
        {           
             
            int k = sp[i].startindex + sp[i].currentindex;
            //printf("value of k is %d \n" , k);
            //getchar();
            if(pr[k].processflag  == 0 && pr[k].scheduled == 'N')
            {
              //printf("Testing");
              int qno = 0;
              for(qno = 0 ; qno<noOfQueues ; qno++)
              {             
                 if(strcmp(q[qno].slavename , pr[k].slavename) == 0)
                 {
                    break;
                 }
               }
               //printf("Scheduled by parent in queue of %d " , qno);
               //getchar();
               //sem_wait(mutex);
               q[qno].prindex[q[qno].rear] = k;
               q[qno].pno[q[qno].rear]  = i;
               q[qno].p[q[qno].rear++] = pr[k];
               pr[k].scheduled = 'Y';
               //sem_post(mutex);          
              //printf("qno = %d , %d %d scheduled\n", qno, q[qno].prindex[q[qno].rear-1], q[qno].pno[q[qno].rear-1]);
               
            }
           
          }
          sem_post(mutex);
         // sleep(1);
    }while(prcontinueflag);

    
    for(i = 0 ; i < cntslv ; i++)
    {        
        //printf(" %d is alive ", pid[i]);
        kill(pid[i], SIGKILL);
        //printf(" %d is killed \n" , pid[i]);
    }
    //printf("Value of cntoutput is %d\noutput %s",*cntoutput, output[0].pname);
    for(i = 0 ; i < *cntoutput ; i++)
    {
      fprintf(fpout , "%s %s : %s %d %d %s\n",output[i].pname, output[i].slavename , output[i].task_type_name , output[i].pid , output[i].waiting , output[i].finishstatus);
    }
    for(i = 0 ; i < cntslv ; i++)
    {
        printf("%d %d %s",fct[i].pid, fct[i].totalnooftasks,fct[i].taskname);
        fprintf(fpout, "%d %d %s",fct[i].pid, fct[i].totalnooftasks,fct[i].taskname);
        for(j=0; j<fct[i].count;j++)
        {
           printf(" %s %d " , fct[i].t[j].taskname, fct[i].t[j].processcounter);
           fprintf(fpout, " %s %d " , fct[i].t[j].taskname, fct[i].t[j].processcounter);
        }
        printf("\n");
        fprintf(fpout , "\n");
    }
    
    shmdt(cntoutput);
    shmdt(output);
    shmdt(tscounter);
    shmdt(mutex);
    shmdt(fct);
    
    shmdt(pr);
    
    shmdt(slv);
    shmdt(tt);
    shmdt(q);
    
    shmctl(shmid0 , IPC_RMID , NULL);
    shmctl(shmidtscounter , IPC_RMID , NULL);
    shmctl(shmidcnt , IPC_RMID , NULL);
    shmctl(shmidoutput , IPC_RMID , NULL);
    
    shmctl(shmid1, IPC_RMID, NULL);
   // shmctl(shmid2, IPC_RMID, NULL);
    shmctl(shmid3, IPC_RMID, NULL);
    
    shmctl(shmid5, IPC_RMID, NULL);
    shmctl(shmid6, IPC_RMID, NULL);
    shmctl(shmid7, IPC_RMID , NULL);
    

}


