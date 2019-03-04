#include <RadarKit.h>

int main(int argc, const char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Please supply a filename.\n");
        return EXIT_FAILURE;
    }

    RKSetWantScreenOutput(true);

    RKProduct *product = RKProductFileReaderNC(argv[1]);
    RKProductFree(product);
    return EXIT_SUCCESS;
}
