#include "libs/raylib.h"
#include "libs/mruby.h"
#include "mrutil.h"

#ifdef _WIN32
    #include<windows.h>
    #define sleep(time) Sleep(time * 1000)
#else
    #include<unistd.h>
#endif

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
            return cmd_run(NULL, script);
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

static const mrb_data_type vector2_type = {
    "Vector2", mrb_free
};

static mrb_value window_open(mrb_state *mrb, mrb_value self) {
    mrb_value block;
    mrb_get_args(mrb, "&", &block);

    if (mrb_nil_p(block)) {
        printf("WARNING: You called Real::Window.open without blocks in arguments! It will cause an infinite loop!\n");
    }

    struct RClass *real_mod = mrb_module_get(mrb, "Real");
    struct RClass *vector2_class = mrb_class_get_under(mrb, real_mod, "Vector2");
    mrb_value size = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@size"));

    if (!mrb_obj_is_kind_of(mrb, size, vector2_class)) {
        mrb_raise(mrb, E_TYPE_ERROR, "expected Real::Vector2");
	return self;
    }

    double *size_data = (double*)mrb_data_get_ptr(mrb, size, &vector2_type);

    InitWindow((int)(size_data[0] + 0.5f), (int)(size_data[1] + 0.5f), "real");
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

    return self;
}

static mrb_value window_close(mrb_state *mrb, mrb_value self) {
    if (IsWindowReady()) CloseWindow();
    return self;
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
    return self;
}

static mrb_value window_set_borderless(mrb_state *mrb, mrb_value self) {
    ToggleBorderlessWindowed();
    return self;
}

static mrb_value window_set_maximize(mrb_state *mrb, mrb_value self) {
    MaximizeWindow();
    return self;
}

static mrb_value window_set_minimize(mrb_state *mrb, mrb_value self) {
    MinimizeWindow();
    return self;
}

static mrb_value window_set_restore(mrb_state *mrb, mrb_value self) {
    RestoreWindow();
    return self;
}

static mrb_value window_get_title(mrb_state *mrb, mrb_value self) {
    mrb_value title = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@title"));
    return mrb_nil_p(title) ? mrb_str_new_lit(mrb, "") : title;
}

static mrb_value window_set_title(mrb_state *mrb, mrb_value self) {
    mrb_value title;
    mrb_get_args(mrb, "S", &title);

    SetWindowTitle(RSTRING_CSTR(mrb, title));

    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@title"), title);

    return title;
}

static mrb_value window_get_size(mrb_state *mrb, mrb_value self) {
    mrb_value size = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@size"));
    return mrb_nil_p(size) ? mrb_str_new_lit(mrb, "") : size;
}

static mrb_value window_get_screen_width(mrb_state *mrb, mrb_value self) {
    int w = GetScreenWidth();
    return mrb_int_value(mrb, w);
}

static mrb_value window_get_screen_height(mrb_state *mrb, mrb_value self) {
    int h = GetScreenHeight();
    return mrb_int_value(mrb, h);
}

static mrb_value window_set_size(mrb_state *mrb, mrb_value self) {
    mrb_value obj;
    mrb_get_args(mrb, "o", &obj);

    struct RClass *real_mod = mrb_module_get(mrb, "Real");
    struct RClass *vector2 = mrb_class_get_under(mrb, real_mod, "Vector2");

    if (!mrb_obj_is_kind_of(mrb, obj, vector2)) {
        mrb_raise(mrb, E_TYPE_ERROR, "expected Real::Vector2");
    }

    double *data = (double*)mrb_data_get_ptr(mrb, obj, &vector2_type);

    if (IsWindowReady())
        SetWindowSize((int)(data[0] + 0.5f), (int)(data[1] + 0.5f));

    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@size"), obj);

    return obj;
}

static mrb_value vector2_init(mrb_state *mrb, mrb_value self) {
    mrb_float x, y;
    mrb_get_args(mrb, "ff", &x, &y);

    double *data = (double*)mrb_malloc(mrb, sizeof(double) * 2);
    data[0] = x;
    data[1] = y;

    DATA_PTR(self) = data;
    DATA_TYPE(self) = &vector2_type;

    return self;
}

static mrb_value vector2_get_x(mrb_state *mrb, mrb_value self) {
    double *data = (double*)mrb_data_get_ptr(mrb, self, &vector2_type);
    return mrb_float_value(mrb, data[0]);
}

static mrb_value vector2_get_y(mrb_state *mrb, mrb_value self) {
    double *data = (double*)mrb_data_get_ptr(mrb, self, &vector2_type);
    return mrb_float_value(mrb, data[1]);
}

static mrb_value vector2_set_x(mrb_state *mrb, mrb_value self) {
    mrb_float x;
    mrb_get_args(mrb, "f", &x);

    double *data = (double*)mrb_data_get_ptr(mrb, self, &vector2_type);
    data[0] = x;

    return self;
}

static mrb_value vector2_set_y(mrb_state *mrb, mrb_value self) {
    mrb_float y;
    mrb_get_args(mrb, "f", &y);

    double *data = (double*)mrb_data_get_ptr(mrb, self, &vector2_type);
    data[1] = y;

    return self;
}

// Sleep for `time` seconds
static mrb_value mrb_sleep(mrb_state *mrb, mrb_value self) {
    mrb_int time;
    mrb_get_args(mrb, "i", &time);
    
    sleep((int)time);

    return mrb_int_value(mrb, time);
}

void load_mruby_objects(mrb_state *mrb) {
    // make sleep global
    mrb_define_method(mrb, mrb->kernel_module, "sleep", mrb_sleep, MRB_ARGS_REQ(1));

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
    mrb_define_class_method(mrb, window, "title", window_get_title, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "title=", window_set_title, MRB_ARGS_REQ(1));
    mrb_define_class_method(mrb, window, "size", window_get_size, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "size=", window_set_size, MRB_ARGS_REQ(1));
    mrb_define_class_method(mrb, window, "screen_width",  window_get_screen_width, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, window, "screen_height", window_get_screen_height, MRB_ARGS_NONE());

    struct RClass *vector2 = mrb_define_class_under(mrb, real_mod, "Vector2", mrb->object_class);
    MRB_SET_INSTANCE_TT(vector2, MRB_TT_DATA);
    mrb_define_method(mrb, vector2, "initialize", vector2_init, MRB_ARGS_REQ(2));
    mrb_define_method(mrb, vector2, "x=", vector2_set_x, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, vector2, "y=", vector2_set_y, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, vector2, "x", vector2_get_x, MRB_ARGS_NONE());
    mrb_define_method(mrb, vector2, "y", vector2_get_y, MRB_ARGS_NONE());
}

int main(int argc, char *argv[]) {
    #ifdef INCLUDE_GAME
        mrb_state *mrb = mrb_open();

        #include "game.h" // mrb bytecode
        load_mruby_objects(mrb);
        mrb_load_irep_buf(mrb, game_main_mrb, game_main_mrb_len);
        mrb_close(mrb);
    #elif DEV_MODE
	mrb_state *mrb = mrb_open();

	load_mruby_objects(mrb);
	cmd_run(mrb, "game/main.rb");
	mrb_close(mrb);
    #else
        if (argc == 1)
            printf("Warning: This version does not have your game inside!\nBut it has some utilities to run and compile mruby scripts.\n\n");
        return mrutil(argc, argv);
    #endif
}
