#include <stdio.h>

#include <stdlib.h>

#include <sys/syscall.h>

#include <time.h>

#include <npheap.h>

#include <fcntl.h>

#include <sys/mman.h>

#include <unistd.h>

#include <string.h>



int main(int argc, char *argv[])

{

    setbuf(stdout,NULL);

    int i=0,number_of_threads = 1, number_of_objects=1024 , max_size_of_objects=8192;

    char buffer[10000];
    FILE *fp;

    int tid;

    long size;

    __u64 object_id;

    __u64 current_time;

    char *data,op,*mapped_data;

    char **obj;

    int devfd;

    int error = 0;

    if(argc < 2)

    {

        fprintf(stderr, "Usage: %s number_of_objects\n",argv[0]);

        exit(1);

    }

    number_of_objects = atoi(argv[1]);

    if(argc > 2)

        max_size_of_objects = atoi(argv[2]);

    data = (char *)malloc(max_size_of_objects*sizeof(char));

    obj = (char **)malloc(number_of_objects*sizeof(char *));

    for(i = 0; i < number_of_objects; i++)

    {

        obj[i] = (char *)calloc(max_size_of_objects, sizeof(char));

    }

    // Replay the log

    // Validate

    // printf("While start\n");

    fp = fopen("sorted_trace","r");

    // while(scanf("%c %d %llu %llu %llu %s",&op, &tid, &current_time, &object_id, &size, &data[0])!=EOF)
    while(fgets(&buffer[0],10000,fp)!=NULL)
    {
        int length = strlen(buffer);
        // printf("Length : %d Char : '%c' Char : '%c' \n",length,buffer[length-1],buffer[length-2]);
        // printf("String : %s\n",buffer);
        if (buffer[length-2]=='\t') {
            // printf("NULL VALUE \n");
            sscanf(&buffer[0],"%c %d %llu %llu %llu",&op, &tid, &current_time, &object_id, &size);
            data[0] = '\0';
        } else {
            // printf("REGULAR \n");
            sscanf(&buffer[0],"%c %d %llu %llu %llu %s",&op, &tid, &current_time, &object_id, &size, &data[0]);
        }

        // printf("%c,%d,%s \n",op,tid,data);

        if(op == 'S')

        {

            strcpy(obj[(int)object_id],data);

            memset(data,0,max_size_of_objects);

        } else if (op == 'G') {

            if (strcmp(obj[(int)object_id], data)) {

                fprintf(stderr, "%d: Key %d has a wrong value %s v.s. %s\n",tid,(int)object_id,data,obj[(int)object_id]);
                fprintf(stderr,"Length(data) %d : Length (reference) %d \n",strlen(data),strlen(obj[(int)object_id]));
                error++;

            }

        }

        else if (op == 'D') {

          // printf("INside D\n");

            memset(obj[(int)object_id],0,max_size_of_objects);

            // printf("Object id %d %s\n", (int)object_id, obj[(int)object_id]);

        }

        if (error > 5) {

            break;

        }

    }

    // printf("OPening device\n" );

    devfd = open("/dev/npheap",O_RDWR);

    if(devfd < 0)

    {

        fprintf(stderr, "Device open failed");

        exit(1);

    }

//    exit(1);

    // printf("I am accesing memory now\n");

    for(i = 0; i < number_of_objects; i++)

    {

        size = npheap_getsize(devfd,i);
        // printf("The size for object %d is %d \n", i, size);
         if(size!=0)
        mapped_data = (char *)npheap_alloc(devfd,i,npheap_getsize(devfd, i));
        else

                    mapped_data = NULL;
        if(strcmp(mapped_data,obj[i])!=0)

        {

            fprintf(stderr, "Object %d has a wrong value %s v.s. %s\n",i,mapped_data,obj[i]);
            fprintf(stderr,"Length(data) %d : Length (reference) %d \n",strlen(mapped_data),strlen(obj[(int)object_id]));

            error++;

        }

    }

    if(error == 0)

        fprintf(stderr,"Pass\n");

    close(devfd);

    return 0;

}
