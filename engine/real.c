#include "libs/raylib.h"
#include "libs/mruby.h"
#include "mrutil.h"

#define VERSION "0.1"

int mrutil(int argc, char *argv[]) {
    if (argc < 3) {
        usage(argv[0]);
        return 1;
    }

    const char *cmd = argv[1];
    const char *script = argv[2];

    if (strcmp(cmd, "run") == 0) {
        size_t len = strlen(script);
        if (len > 4 && strcmp(script + len - 4, ".mrb") == 0) {
            return cmd_run_mrb(script);
        } else {
            return cmd_run(script);
        }
    } else if (strcmp(cmd, "compile") == 0) {
        char *out_path = NULL;
        int free_out   = 0;

        if (argc >= 5 && strcmp(argv[3], "-o") == 0) {
            out_path = (char *)argv[4];
        } else {
            out_path = default_output_path(script);
            free_out = 1;
        }

        int result = cmd_compile(script, out_path);
        if (free_out) free(out_path);
        return result;
    } else {
        fprintf(stderr, "Unknown command: '%s'\n", cmd);
        usage(argv[0]);
        return 1;
    }
}

static mrb_value window_open(mrb_state *mrb, mrb_value self) {
    mrb_value block;
    mrb_get_args(mrb, "&", &block);

    if (mrb_nil_p(block)) {
        printf("WARNING: You called Real::Window.open without blocks in arguments! It will cause an infinite loop!\n");
    }

    InitWindow(500, 500, "real");
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        BeginDrawing();

        if (!mrb_nil_p(block)) {
            mrb_yield(mrb, block, mrb_nil_value());
        }

        if (!IsWindowReady()) break;

        EndDrawing();
    }

    if (IsWindowReady()) CloseWindow();

    return mrb_nil_value();
}

static mrb_value window_close(mrb_state *mrb, mrb_value self) {
    if (IsWindowReady()) CloseWindow();
    return mrb_nil_value();
}

static mrb_value window_is_ready(mrb_state *mrb, mrb_value self) {
    return IsWindowReady() ? mrb_true_value() : mrb_false_value();
}

static mrb_value window_is_fullscreen(mrb_state *mrb, mrb_value self) {
    return IsWindowFullscreen() ? mrb_true_value() : mrb_false_value();
}

static mrb_value window_is_hidden(mrb_state *mrb, mrb_value self) {
    return IsWindowHidden() ? mrb_true_value() : mrb_false_value();
}

static mrb_value window_is_minimized(mrb_state *mrb, mrb_value self) {
    return IsWindowMinimized() ? mrb_true_value() : mrb_false_value();
}

static mrb_value window_is_maximized(mrb_state *mrb, mrb_value self) {
    return IsWindowMaximized() ? mrb_true_value() : mrb_false_value();
}

static mrb_value window_is_focused(mrb_state *mrb, mrb_value self) {
    return IsWindowFocused() ? mrb_true_value() : mrb_false_value();
}

static mrb_value window_is_resized(mrb_state *mrb, mrb_value self) {
    return IsWindowResized() ? mrb_true_value() : mrb_false_value();
}

static mrb_value window_set_fullscreen(mrb_state *mrb, mrb_value self) {
    ToggleFullscreen();
    return mrb_nil_value();
}

static mrb_value window_set_borderless(mrb_state *mrb, mrb_value self) {
    ToggleBorderlessWindowed();
    return mrb_nil_value();
}

static mrb_value window_set_maximize(mrb_state *mrb, mrb_value self) {
    MaximizeWindow();
    return mrb_nil_value();
}

static mrb_value window_set_minimize(mrb_state *mrb, mrb_value self) {
    MinimizeWindow();
    return mrb_nil_value();
}

static mrb_value window_set_restore(mrb_state *mrb, mrb_value self) {
    RestoreWindow();
    return mrb_nil_value();
}

void load_mruby_objects(mrb_state *mrb) {
    struct RClass *real_mod = mrb_define_module(mrb, "Real");

    mrb_define_const(mrb, real_mod, "VERSION", mrb_str_new_lit(mrb, VERSION));

    struct RClass *window = mrb_define_class_under(mrb, real_mod, "Window", mrb->object_class);
    mrb_define_class_method(mrb, window, "open", window_open, MRB_ARGS_BLOCK());
    mrb_define_class_method(mrb, window, "close", window_close, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "ready?", window_is_ready, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "fullscreen?", window_is_fullscreen, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "hidden?", window_is_hidden, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "minimized?", window_is_minimized, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "maximized?", window_is_maximized, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "minimize?", window_is_minimized, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "maximize?", window_is_maximized, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "focused?", window_is_focused, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "focus?", window_is_focused, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "resized?", window_is_resized, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "resize?", window_is_resized, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "fullscreen", window_set_fullscreen, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "borderless", window_set_borderless, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "minimize", window_set_minimize, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "maximize", window_set_maximize, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "restore", window_set_restore, MRB_ARGS_NONE());
}

int main(int argc, char *argv[]) {
    #ifdef INCLUDE_GAME
        mrb_state *mrb = mrb_open();

        #include "game.h"
        load_mruby_objects(mrb);
        mrb_load_irep_buf(mrb, game_main_mrb, game_main_mrb_len);
        mrb_close(mrb);
    #else
        if (argc == 1)
            printf("Warning: This version does not have your game inside!\nBut it has some utilities to run and compile mruby scripts.\n\n");
        return mrutil(argc, argv);
    #endif
}
