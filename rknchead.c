#include <RadarKit.h>

int main(int argc, const char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Please supply a filename.\n");
        return EXIT_FAILURE;
    }

    RKSetWantScreenOutput(true);
    
    RKProductCollection *collection = RKProductCollectionInitWithFilename(argv[1]);

    RKProductCollectionFree(collection);
    
    return EXIT_SUCCESS;
}
