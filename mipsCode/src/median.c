#define DEBUG

#ifdef DEBUG
   #include <stdio.h>
#endif

//Compile using: gcc -std=c99 median.c -o median


typedef unsigned char T;

#define N 10
static T output[1+N+1][1+N+1],
         input[1+N+1][1+N+1] =
      {
         {0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,    0},
         {0,   2,149,171,216, 22, 38,137, 66, 65,249,    0},
         {0, 208,216, 94,143,140, 88, 77,243, 90, 34,    0},
         {0,  15,132, 11,143,169, 20, 70, 83, 44, 15,    0},
         {0, 172, 37,159, 52,105,180, 41, 81, 47,102,    0},
         {0,  98,192,  5,209,115,130,190,205, 38, 48,    0},
         {0, 189,247,210,213,222, 37,166,133,158,254,    0},
         {0, 168, 79, 52,208,132, 85,  1,137, 44,161,    0},
         {0,  57, 54,251, 39,177,211,170,253,  4, 89,    0},
         {0, 251,206,232, 91, 94, 32,150,147,127,195,    0},
         {0,  26,246, 16,130, 93, 75, 28, 50, 31,253,    0},
         {0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,    0}
      };

#define BUBBLESORT
//#define MINCOMP
//#define MINTIME

#define ASCENDING
#define BRANCHLESS

//TODO! Check branchless correctness more thoroughly
#ifdef ASCENDING
   #ifdef BRANCHLESS
      #define CAS(x,y) tmp=y ^ ((x ^ y) & -(x < y));  /*min(x, y)*/ \
                       y=x ^ ((x ^ y) & -(x < y));    /*max(x, y)*/ \
                       x=tmp;
   #else
      #define CAS(x,y) tmp=(x>y ? y : x); \
                       y=(x>y ? x : y); \
                       x=tmp;
   #endif
#else
   #ifdef BRANCHLESS
      #define CAS(x,y) tmp=x ^ ((x ^ y) & -(x < y));  /*max(x, y)*/ \
                       y=y ^ ((x ^ y) & -(x < y));    /*min(x, y)*/ \
                       x=tmp;
   #else
      #define CAS(x,y) tmp=(x<y ? y : x); \
                       y=(x<y ? x : y); \
                       x=tmp;
   #endif
#endif

#define OFFSET(x, y) (x)*(N+2)+(y)

void median(T *Min, T *Mout) {
   register T tmp = 0;
    unsigned int r;
    unsigned int c;
   for(r = 1; r < N+1; r++)
       for(c = 1; c < N+1; c++) {
           register T v00 = Min[OFFSET(r-1,c-1)], v01 = Min[OFFSET(r-1,  c)], v02 = Min[OFFSET(r-1,c+1)],
                      v10 = Min[OFFSET(r  ,c-1)], v11 = Min[OFFSET(r  ,  c)], v12 = Min[OFFSET(r  ,c+1)],
                      v20 = Min[OFFSET(r+1,c-1)], v21 = Min[OFFSET(r+1,  c)], v22 = Min[OFFSET(r+1,c+1)];
           #if defined(BUBBLESORT)
               CAS(v00, v01)
               CAS(v01, v02)
               CAS(v02, v10)
               CAS(v10, v11)
               CAS(v11, v12)
               CAS(v12, v20)
               CAS(v20, v21)
               CAS(v21, v22)

               CAS(v00, v01)
               CAS(v01, v02)
               CAS(v02, v10)
               CAS(v10, v11)
               CAS(v11, v12)
               CAS(v12, v20)
               CAS(v20, v21)

               CAS(v00, v01)
               CAS(v01, v02)
               CAS(v02, v10)
               CAS(v10, v11)
               CAS(v11, v12)
               CAS(v12, v20)

               CAS(v00, v01)
               CAS(v01, v02)
               CAS(v02, v10)
               CAS(v10, v11)
               CAS(v11, v12)

               CAS(v00, v01)
               CAS(v01, v02)
               CAS(v02, v10)
               CAS(v10, v11)

               CAS(v00, v01)
               CAS(v01, v02)
               CAS(v02, v10)

               CAS(v00, v01)
               CAS(v01, v02)

               CAS(v00, v01)
          #elif defined(MINCOMP)
               CAS(v00, v01)    CAS(v10, v11)    CAS(v20, v21)
               CAS(v01, v02)    CAS(v11, v12)    CAS(v21, v22)
               CAS(v00, v01)    CAS(v10, v11)    CAS(v20, v21)
               CAS(v00, v10)
               CAS(v10, v20)
               CAS(v00, v10)
               CAS(v01, v11)
               CAS(v11, v21)
               CAS(v01, v11)
               CAS(v02, v12)
               CAS(v12, v22)
               CAS(v02, v12)
               CAS(v01, v10)    CAS(v12, v21)
               CAS(v02, v20)
               CAS(v11, v20)
               CAS(v02, v11)
               CAS(v02, v10)    CAS(v12, v20)
            #elif defined(MINTIME)
               CAS(v01, v02)    CAS(v10, v11)    CAS(v12, v20)    CAS(v21, v22)
               CAS(v00, v01)    CAS(v02, v11)    CAS(v20, v22)
               CAS(v12, v21)
               CAS(v00, v10)    CAS(v11, v22)
               CAS(v01, v02)    CAS(v20, v21)
               CAS(v10, v21)
               CAS(v01, v20)
               CAS(v00, v12)
               CAS(v02, v11)
               CAS(v11, v21)
               CAS(v02, v20)
               CAS(v10, v12)
               CAS(v01, v10)    CAS(v11, v20)    CAS(v21, v22)
               CAS(v02, v12)
               CAS(v02, v10)    CAS(v11, v12)
            #endif

            Mout[OFFSET(r,c)] = v11;
       }
}

int main(int argc, char *argv[]) {
   median(&input[0][0], &output[0][0]);

   #ifdef DEBUG
      printf("Input matrix:\n");
        unsigned int i;
        unsigned int j;
      for(i = 0; i < N+2; i++) {
          printf("{");
          for(j = 0; j < N+2; j++) {
              if(j != 0)
                 printf(", ");

              if(input[i][j] >= 100)
                 printf("%d", input[i][j]);
              else if(input[i][j] >= 10)
                 printf(" %d", input[i][j]);
              else
                 printf("  %d", input[i][j]);
          }
          printf("}\n");
      }
      printf("Output matrix:\n");
      for(i = 0; i < N+2; i++) {
          printf("{");
          for(j = 0; j < N+2; j++) {
              if(j != 0)
                 printf(", ");

              if(output[i][j] >= 100)
                 printf("%d", output[i][j]);
              else if(output[i][j] >= 10)
                 printf(" %d", output[i][j]);
              else
                 printf("  %d", output[i][j]);
          }
          printf("}\n");
      }
   #endif
}

