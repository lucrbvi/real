const std = @import("std");

const c_sources: []const []const u8 = &.{
    "engine/real.c",
    "engine/libs/mruby.c",
};

fn linkPlatformLibs(exe: *std.Build.Step.Compile, os: std.Target.Os.Tag) void {
    switch (os) {
        .driverkit, .ios, .macos, .tvos, .visionos, .watchos => {
            for ([_][]const u8{
                "Cocoa", "OpenGL", "IOKit",
                "CoreVideo", "CoreFoundation",
                "CoreGraphics", "AppKit",
            }) |fw| exe.linkFramework(fw);
        },
        .windows => {
            for ([_][]const u8{
               
            }) |fw| exe.linkFramework(fw);
        },
        else => {
            for ([_][]const u8{ "m", "pthread" }) |lib|
                exe.linkSystemLibrary(lib);
        },
    }
}

fn addEngineExe(
    b: *std.Build,
    name: []const u8,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    flags: []const []const u8,
) *std.Build.Step.Compile {
    const platform_flags: []const []const u8 = switch (target.result.os.tag) {
        .windows => &.{
            "-DMRB_USE_SETJMP_H",
            "-DMRB_NO_PROCESS",
            "-D_WIN32_WINNT=0x0601",
            "-Disatty=_isatty",
            "-Wno-macro-redefined",
        },
        else => &.{},
    };

    const all_flags = std.mem.concat(b.allocator, []const u8, &.{
        flags,
        platform_flags,
    }) catch @panic("OOM");

    const exe = b.addExecutable(.{
        .name = name,
        .root_module = b.createModule(.{ .target = target, .optimize = optimize }),
    });
    exe.root_module.addCSourceFiles(.{ .files = c_sources, .flags = all_flags });
    exe.root_module.addIncludePath(b.path("engine/libs"));
    exe.linkLibC();
    exe.addLibraryPath(b.path("engine/libs"));
    exe.linkSystemLibrary("raylib");
    linkPlatformLibs(exe, target.result.os.tag);
    return exe;
}

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // Replace xxd 
    const bin2c = b.addExecutable(.{
        .name = "bin2c",
        .root_module = b.createModule(.{
            .root_source_file = b.path("utils/bin2c.zig"),
            .target = b.graph.host,
        }),
    });

    // used to compile Ruby files to mruby bytecode (not a great idea to bundle the whole engine for that) 
    const real_host = addEngineExe(b, "real_host", b.graph.host, .ReleaseFast, &.{});

    // game/main.rb -> main.mrb
    const compile_mrb = b.addRunArtifact(real_host);
    compile_mrb.stdio = .inherit;
    compile_mrb.addArg("compile");
    compile_mrb.addFileArg(b.path("game/main.rb"));
    compile_mrb.addArg("-o");
    const mrb_file = compile_mrb.addOutputFileArg("main.mrb");

    // main.mrb -> game.h
    const run_bin2c = b.addRunArtifact(bin2c);
    run_bin2c.addFileArg(mrb_file);
    const write_files = b.addWriteFiles();
    const game_h_dir = write_files.addCopyFile(run_bin2c.captureStdOut(), "game.h").dirname();

    const game = addEngineExe(b, "game", target, optimize, &.{"-DINCLUDE_GAME"});
    game.root_module.addIncludePath(game_h_dir);
    b.installArtifact(game);


    const run_cmd = b.addRunArtifact(game);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| run_cmd.addArgs(args);
    b.step("run", "Run the game").dependOn(&run_cmd.step);
}
