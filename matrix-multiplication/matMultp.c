#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/time.h>

#define MAXLINE 1024 //maximum chars in a line
#define MAXNUM 6 //maximum number length (-99999 <= number <= 999999)
#define MAXROWS 100
#define MAXCOLS 100

int mat1[MAXROWS][MAXCOLS];
int mat2[MAXROWS][MAXCOLS];
int result[MAXROWS][MAXCOLS];

//main functions
void matrixThread(int l, int m, int n);
void rowThread(int l, int m, int n);
void elementThread(int l, int m, int n);

void* rowRoutine(void* arg);
void* elementRoutine(void* arg);

//file functions
void findRowsCols(char* fileName, int RowsCols[2]);
void readMatrix(char *fileName, int r, int c, int mat[MAXROWS][MAXCOLS]);
void writeMatrix(char* fileName, int r, int c, int mat[MAXROWS][MAXCOLS], char* message);

int main(int argc, char* argv[]) {
    /////////////handle arguments/////////////
    char* f1 = (argc > 1) ? argv[1] : "a.txt";
    char* f2 = (argc > 2) ? argv[2] : "b.txt";
    char* f3 = (argc > 3) ? argv[3] : "c.out";

    /////////////read matrices from files/////////////
    int d1[2], d2[2];
    //read first matrix
    findRowsCols(f1, d1);
    if(d1[0] > MAXROWS || d1[1] > MAXCOLS){
        printf("matrix size limits exceeded maximum allowed size: maxrows = %d, maxcols = %d\n", MAXROWS, MAXCOLS); 
        exit(1);
    }
    readMatrix(f1, d1[0], d1[1], mat1);
    //read second matrix
    findRowsCols(f2, d2);
    if(d2[0] > MAXROWS || d2[1] > MAXCOLS){
        printf("matrix size limits exceeded maximum allowed size: maxrows = %d, maxcols = %d\n", MAXROWS, MAXCOLS); 
        exit(1);
    }
    readMatrix(f2, d2[0], d2[1], mat2);

    /////////////multiply matrices/////////////
    if(d1[1] != d2[0]) {
        puts("matrices dimensions error, first matrix columns should equal second matrix rows"); 
        exit(1);
    }
    struct timeval stop, start; //for calculating runtime

    //method 1
    gettimeofday(&start, NULL);
    matrixThread(d1[0], d1[1], d2[1]);
    gettimeofday(&stop, NULL);
    long t1 = stop.tv_usec - start.tv_usec;
    //method 2
    gettimeofday(&start, NULL);
    rowThread(d1[0], d1[1], d2[1]);
    gettimeofday(&stop, NULL);
    long t2 = stop.tv_usec - start.tv_usec;
    //method 3
    gettimeofday(&start, NULL);
    elementThread(d1[0], d1[1], d2[1]);
    gettimeofday(&stop, NULL);
    long t3 = stop.tv_usec - start.tv_usec;
    
    char message[256]; //this message will be printed at the end of the file and in stdout
    sprintf(message, "\t\tsingle thread:\ntime = %lu\t\tnumber of threads = 1\n"
            "\t\tthread per row:\ntime = %lu\t\tnumber of threads = %d\n"
            "\t\tthread per element:\ntime = %lu\t\tnumber of threads = %d"
            , t1, t2, d1[0], t3, d1[0]*d2[1]);
    //write result and the message in the file f3
    writeMatrix(f3, d1[0], d2[1], result, message);
    puts(message);
    
    return 0;
}

void matrixThread(int l, int m, int n) {
    for(int i=0; i<l; i++) { //loop throw first matrix rows
        for(int j=0; j<n; j++) { //loop throw second matrix columns
            int sum = 0;
            for(int k=0; k<m; k++) {
                sum += mat1[i][k] * mat2[k][j];
            }
            result[i][j] = sum;
        }
    }
}

void rowThread(int l, int m, int n) {
    pthread_t thread[l]; int data[l][3];
    for(int i=0; i<l; i++) {
        data[i][0] = i; data[i][1] = m; data[i][2] = n;
        //thread per row
        pthread_create(&thread[i], NULL, rowRoutine, (void*) data[i]);
    }
    //wait for child threads to end
    for(int i=0; i<l; i++) pthread_join(thread[i], NULL);
}
void* rowRoutine(void* arg) {
    int* data = (int*) arg;
    for(int j=0; j<data[2]; j++) {
        int sum = 0;
        for(int k=0; k<data[1]; k++) {
            sum += mat1[data[0]][k] * mat2[k][j];
        }
        result[data[0]][j] = sum;
    }
    pthread_exit(NULL);
}

void elementThread(int l, int m, int n) {
    pthread_t thread[l*n]; int data[l*n][3], count = 0;
    for(int i=0; i<l; i++) {
        for(int j=0; j<n; j++) {
            data[count][0] = i; data[count][1] = m; data[count][2] = j;
            //thread per element
            pthread_create(&thread[count], NULL, elementRoutine, (void*) data[count]);
            count++;
        }
    }
    //wait for child threads to end
    for(int i=0; i<count; i++) pthread_join(thread[i], NULL);
}
void* elementRoutine(void* arg) {
    int* data = (int*) arg;
    int sum = 0;
    for(int k=0; k<data[1]; k++) {
        sum += mat1[data[0]][k] * mat2[k][data[2]];
    }
    result[data[0]][data[2]] = sum;
    pthread_exit(NULL);
}

/********************* file functions *********************/

void findRowsCols(char* fileName, int RowsCols[2]) {
    FILE *fp = fopen(fileName, "r");
    if(fp == NULL) { puts("file not found"); exit(1);}
    
    char line[MAXLINE];
    if(fgets(line, MAXLINE, fp) == NULL) {puts("empty file"); exit(1);}
    //split by space
    char* row = strtok(line, " ");
    char* col = strtok(NULL, " ");
    //split by "="
    strtok(row, "=");
    char* numRows = strtok(NULL, "=");
    strtok(col, "=");
    char* numCols = strtok(NULL, "=");
    //convert to int
    RowsCols[0] = atoi(numRows);
    RowsCols[1] = atoi(numCols);
    //check if number of rows/columns is vaild
    if(RowsCols[0] <=0 || RowsCols[1] <=0) {
        printf("invaild row/column number: row=%s col=%s", numRows, numCols);
        exit(1);
    }
    fclose(fp);
}

void readMatrix(char *fileName, int r, int c, int mat[MAXROWS][MAXCOLS]) {
    FILE *fp = fopen(fileName, "r");
    if(fp == NULL) { puts("file not found"); exit(1);}

    char line[MAXLINE];
    fgets(line, MAXLINE, fp); // escape line which contains "row=x col=y"
    
    int i = 0;
    while(fgets(line, MAXLINE, fp) != NULL) { //loop throw lines
        int j = 0;
        char* num = strtok(line, "\t"); //read line untill tap found
        while(num != NULL) { //contiue reading until line ended
            //safety check to avoid dummy users
            if(isdigit(num[0]) || (num[0]=='-' && isdigit(num[1])))
                mat[i][j++] = atoi(num);
            num = strtok(NULL, "\t");
        }
        //if readed columns not equal to expected columns
        if(j!=c) {
            printf("error reading columns at line %d,"
            " expected %d elements but %d was found\n",i+2, c, j); 
            exit(1);
        }
        i++;
    }
    //if readed rows not equal to expected rows
    if(i!=r) {puts("error reading rows"); exit(1);}
    fclose(fp);
}

void writeMatrix(char* fileName, int r, int c, int mat[MAXROWS][MAXCOLS], char* message) {
    FILE* fp = fopen(fileName, "w");
    for(int i=0; i<r; i++) {
        for(int j=0; j<c; j++) {
            fprintf(fp, "%d ", mat[i][j]);
        }
        fprintf(fp, "\n");
    }
    //print the message at the end of the file
    fprintf(fp, "%s",message);
    fclose(fp);
}