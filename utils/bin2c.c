#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stdout, "usage: bin2c <input>\n");
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        fprintf(stderr, "error: cannot open file '%s'\n", argv[1]);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    if (size > 100 * 1024 * 1024) {
        fprintf(stderr, "error: file too large\n");
        fclose(f);
        return 1;
    }

    unsigned char *data = malloc(size);
    if (!data) {
        fprintf(stderr, "error: out of memory\n");
        fclose(f);
        return 1;
    }

    if ((long)fread(data, 1, size, f) != size) {
        fprintf(stderr, "error: failed to read file\n");
        free(data);
        fclose(f);
        return 1;
    }
    fclose(f);

    printf("unsigned char game_main_mrb[] = {\n");
    for (long i = 0; i < size; i++) {
        if (i % 12 == 0) printf(" ");
        printf("0x%02x", data[i]);
        if (i < size - 1) printf(", ");
        if ((i + 1) % 12 == 0) printf("\n");
    }
    printf("\n};\nunsigned int game_main_mrb_len = %ld;\n", (long)size);

    free(data);
    return 0;
}
