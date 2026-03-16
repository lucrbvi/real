const std = @import("std");

const c_sources: []const []const u8 = &.{
    "engine/real.c",
    "engine/libs/mruby.c",
};

fn linkPlatformLibs(exe: *std.Build.Step.Compile, os: std.Target.Os.Tag) void {
    switch (os) {
        .driverkit, .ios, .macos, .tvos, .visionos, .watchos => {
            for ([_][]const u8{
                "Cocoa",     "OpenGL",         "IOKit",
                "CoreVideo", "CoreFoundation", "CoreGraphics",
                "AppKit",
            }) |fw| exe.root_module.linkFramework(fw, .{});
        },
        .windows => {
            for ([_][]const u8{
                "winmm", "gdi32", "opengl32",
            }) |lib| exe.root_module.linkSystemLibrary(lib, .{});
        },
        else => {
            for ([_][]const u8{ "m", "pthread" }) |lib|
                exe.root_module.linkSystemLibrary(lib, .{});
        },
    }
}

fn buildRaylib(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
) *std.Build.Step.Compile {
    const raylib_dep = b.dependency("raylib", .{
        .target = target,
        .optimize = optimize,
    });
    return raylib_dep.artifact("raylib");
}

fn addEngineExe(
    b: *std.Build,
    name: []const u8,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    flags: []const []const u8,
    raylib: *std.Build.Step.Compile,
) *std.Build.Step.Compile {
    const platform_flags: []const []const u8 = switch (target.result.os.tag) {
        .windows => &.{
            "-DMRB_USE_SETJMP_H",
            "-DMRB_NO_PROCESS",
            "-DMRB_NO_IO",
            "-D_WIN32_WINNT=0x0601",
            "-DWIN32_LEAN_AND_MEAN",
            "-D_WINSOCKAPI_",
            "-Disatty=_isatty",
            "-Wno-macro-redefined",
            "-Wno-incompatible-pointer-types",
        },
        else => &.{},
    };

    const all_flags = std.mem.concat(
        b.allocator,
        []const u8,
        &.{ flags, platform_flags },
    ) catch @panic("OOM");

    const exe = b.addExecutable(.{
        .name = name,
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });
    exe.root_module.addCSourceFiles(.{ .files = c_sources, .flags = all_flags });
    exe.root_module.addIncludePath(b.path("engine/libs"));
    exe.root_module.link_libc = true;

    exe.root_module.linkLibrary(raylib);
    exe.root_module.addIncludePath(
        raylib.getEmittedIncludeTree(),
    );

    linkPlatformLibs(exe, target.result.os.tag);
    return exe;
}

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const bin2c = b.addExecutable(.{
        .name = "bin2c",
        .root_module = b.createModule(.{
            .target = b.graph.host,
        }),
    });
    bin2c.root_module.addCSourceFiles(.{
        .files = &.{"utils/bin2c.c"},
        .flags = &.{},
    });
    bin2c.root_module.link_libc = true;

    const raylib_host = buildRaylib(b, b.graph.host, .ReleaseFast);
    const raylib_target = buildRaylib(b, target, optimize);

    const real_host = addEngineExe(
        b,
        "real_host",
        b.graph.host,
        .ReleaseFast,
        &.{},
        raylib_host,
    );

    const compile_mrb = b.addRunArtifact(real_host);
    compile_mrb.stdio = .inherit;
    compile_mrb.addArg("compile");
    compile_mrb.addFileArg(b.path("game/main.rb"));
    compile_mrb.addArg("-o");
    const mrb_file = compile_mrb.addOutputFileArg("main.mrb");

    const run_bin2c = b.addRunArtifact(bin2c);
    run_bin2c.addFileArg(mrb_file);
    const write_files = b.addWriteFiles();
    const game_h_dir = write_files
        .addCopyFile(run_bin2c.captureStdOut(.{}), "game.h")
        .dirname();

    const game = addEngineExe(
        b,
        "game",
        target,
        optimize,
        &.{"-DINCLUDE_GAME"},
        raylib_target,
    );
    game.root_module.addIncludePath(game_h_dir);
    b.installArtifact(game);

    const run_cmd = b.addRunArtifact(game);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| run_cmd.addArgs(args);
    b.step("run", "Run the game").dependOn(&run_cmd.step);
}
