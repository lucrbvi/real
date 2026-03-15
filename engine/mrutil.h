// Some utilities to run and compile mruby scripts

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs/mruby.h"

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage:\n"
        "  %s run <script.rb>\n"
        "  %s compile <script.rb> [-o <output.mrb>]\n",
        prog, prog
    );
}

static char *default_output_path(const char *input) {
    size_t len = strlen(input);
    char *out = (char *)malloc(len + 5);
    strcpy(out, input);

    if (len > 3 && strcmp(out + len - 3, ".rb") == 0) {
        strcpy(out + len - 3, ".mrb");
    } else {
        strcat(out, ".mrb");
    }
    return out;
}

static int cmd_run(const char *path) {
    mrb_state *mrb = mrb_open();
    if (!mrb) {
        fprintf(stderr, "Allocation error\n");
        return 1;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "File error: cannot open '%s'\n", path);
        mrb_close(mrb);
        return 1;
    }

    mrbc_context *ctx = mrbc_context_new(mrb);
    mrbc_filename(mrb, ctx, path);

    mrb_load_file_cxt(mrb, f, ctx);
    fclose(f);
    mrbc_context_free(mrb, ctx);

    int exit_code = 0;
    if (mrb->exc) {
        mrb_print_error(mrb);
        exit_code = 1;
    }

    mrb_close(mrb);
    return exit_code;
}

static int cmd_compile(const char *path, const char *out_path) {
    mrb_state *mrb = mrb_open();
    if (!mrb) {
        fprintf(stderr, "Allocation error\n");
        return 1;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "File error: cannot open '%s'\n", path);
        mrb_close(mrb);
        return 1;
    }

    mrbc_context *ctx = mrbc_context_new(mrb);
    mrbc_filename(mrb, ctx, path);

    struct mrb_parser_state *parser = mrb_parse_file(mrb, f, ctx);
    fclose(f);
    mrbc_context_free(mrb, ctx);

    if (!parser || parser->nerr > 0) {
        fprintf(stderr, "Parsing error in '%s'\n", path);
        if (parser) mrb_parser_free(parser);
        mrb_close(mrb);
        return 1;
    }

    struct RProc *proc = mrb_generate_code(mrb, parser);
    mrb_parser_free(parser);

    if (!proc) {
        fprintf(stderr, "Error: bytecode generation failed\n");
        mrb_close(mrb);
        return 1;
    }

    FILE *out = fopen(out_path, "wb");
    if (!out) {
        fprintf(stderr, "File error: cannot write in '%s'\n", out_path);
        mrb_close(mrb);
        return 1;
    }

    int result = mrb_dump_irep_binary(mrb, proc->body.irep, 0, out);
    fclose(out);

    if (result != MRB_DUMP_OK) {
        fprintf(stderr, "Error: failed dump (code %d)\n", result);
        mrb_close(mrb);
        return 1;
    }

    mrb_close(mrb);
    return 0;
}

static int cmd_run_mrb(const char *path) {
    mrb_state *mrb = mrb_open();
    if (!mrb) {
        fprintf(stderr, "Allocation error\n");
        return 1;
    }

    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "File error: cannot open '%s'\n", path);
        mrb_close(mrb);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    uint8_t *buf = (uint8_t *)malloc(size);
    fread(buf, 1, size, f);
    fclose(f);

    mrb_load_irep_buf(mrb, buf, size);
    free(buf);

    int exit_code = 0;
    if (mrb->exc) {
        mrb_print_error(mrb);
        exit_code = 1;
    }

    mrb_close(mrb);
    return exit_code;
}
