#include <RadarKit.h>

int main(int argc, const char **argv) {

    if (argc < 2) {
        fprintf(stderr, "Please supply a filename.\n");
        return EXIT_FAILURE;
    }

    char filename[1024];
    struct timeval s, e;

    strcpy(filename, argv[1]);

    RKSetWantScreenOutput(true);

    int k = 0;
    gettimeofday(&s, NULL);
    for (int i = 0; i < 500; i++) {
        printf("Trial #%04d   Filename = %s\n", i, filename);
        RKProductCollection *collection = RKProductCollectionInitWithFilename(filename);
        k += collection->count;
        RKProductCollectionFree(collection);
    }
    gettimeofday(&e, NULL);
    double dt = RKTimevalDiff(e, s);
    RKLog("Elapsed time = %.3f s   (%s files / sec)\n", dt, RKFloatToCommaStyleString((double)k / dt));

    return EXIT_SUCCESS;
}
