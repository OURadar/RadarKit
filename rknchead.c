#include <RadarKit.h>

int main(int argc, const char **argv) {

    if (argc < 2) {
        fprintf(stderr, "Please supply a filename.\n");
        return EXIT_FAILURE;
    }

    RKSetWantScreenOutput(true);
    
    char filename[1024];
    strcpy(filename, argv[1]);
    
    for (int i = 0; i < 10; i++) {
    
        printf("Filename = %s\n", filename);
        
        RKProductCollection *collection = RKProductCollectionInitWithFilename(filename);

        RKProductCollectionFree(collection);
    
        sleep(1);
    }
    
    return EXIT_SUCCESS;
}
