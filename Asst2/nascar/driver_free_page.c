
#include "my_pthread_t.h"
#include <string.h>

int main(){
		char *x = mymalloc(24500);
        int i = 0;
        printf("Before Free X:\n");
        for(i = 0; i < 6; i++){
            page * ptr = PT->pages[i];
            printf("%d\n",ptr->is_initialized);
        }
        my_free(x);
        printf("After Free X:\n");
        for(i = 0; i < 6; i++){
            page * ptr = PT->pages[i];
            printf("%d\n",ptr->is_initialized);
        }
		char *y = mymalloc(4000);
        printf("New Malloced Y:\n");
        for(i = 0; i < 6; i++){
            page * ptr = PT->pages[i];
            printf("%d\n",ptr->is_initialized);
        }

}
