
#include "my_pthread_t.h"
#include <string.h>

int main(){
/*		char *x = mymalloc(24500);
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
*/

	char * x = mymalloc(3);
	char *y = mymalloc(1);
	
	printf("\n\n\nx address: %p, x metadata: %d, y address: %p, y metadata: %d\n", x-4, *(int *)(x-4), y-4, *(int *) (y-4));
	
	my_free(x);
	char * z = mymalloc(1);
	printf("z address: %p, z metadata: %d\n", z-4, *(int *)(z-4));
	my_free(y);
	char * a = mymalloc(1);
	printf ("a address: %p, a metadata: %d\n", a-4, *(int *)(a-4));


}
