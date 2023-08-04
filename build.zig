const std = @import("std");

fn getCSourceFiles(b: *std.build.Builder) !std.ArrayList([]const u8) {
    const directory = "src";

    var absolute_path = try std.fs.path.join(b.allocator, &.{ b.build_root, directory });

    var iterable_directory = try std.fs.openIterableDirAbsolute(absolute_path, .{});

    var walker = try iterable_directory.walk(b.allocator);
    defer walker.deinit();

    var source_files_paths = std.ArrayList([]const u8).init(b.allocator);

    while (try walker.next()) |item| {
        if (item.kind != .File) {
            continue;
        }

        var extension = std.fs.path.extension(item.basename);

        if (!std.mem.eql(u8, ".c", extension)) {
            continue;
        }

        var partial = try b.allocator.dupe(u8, item.path);
        var path = try std.fs.path.join(b.allocator, &.{ directory, partial });

        try source_files_paths.append(path);
    }

    return source_files_paths;
}

pub fn build(b: *std.build.Builder) !void {
    var sources = try getCSourceFiles(b);
    defer sources.deinit();

    const target = b.standardTargetOptions(.{});
    const mode = b.standardReleaseOptions();

    const exe = b.addExecutable("ltlr", null);

    exe.setTarget(target);
    exe.setBuildMode(mode);

    if (exe.build_mode != .Debug) {
        exe.strip = true;
    }

    exe.linkLibC();

    exe.addCSourceFiles(sources.items, &.{
        "-std=gnu17",
        "-DPLATFORM_DESKTOP",
        "-Ivendor/raylib/src",
        "-Ivendor/wyhash",
    });

    switch (target.getOsTag()) {
        .windows => {
            if (exe.build_mode != .Debug) {
                exe.subsystem = .Windows;
            }

            exe.addCSourceFiles(&.{
                "vendor/raylib/src/raudio.c",
                "vendor/raylib/src/rcore.c",
                "vendor/raylib/src/rglfw.c",
                "vendor/raylib/src/rshapes.c",
                "vendor/raylib/src/rtext.c",
                "vendor/raylib/src/rtextures.c",
                "vendor/raylib/src/utils.c",
            }, &.{
                "-std=gnu99",
                "-D_GNU_SOURCE",
                "-DPLATFORM_DESKTOP",
                "-DGRAPHICS_API_OPENGL_33",
                "-Ivendor/raylib/src/external/glfw/include",
                "-Ivendor/raylib/src/external/glfw/deps/mingw",
                "-fno-sanitize=undefined",
            });

            exe.linkSystemLibrary("opengl32");
            exe.linkSystemLibrary("gdi32");
            exe.linkSystemLibrary("winmm");
        },
        .linux => {
            exe.addCSourceFiles(&.{
                "vendor/raylib/src/raudio.c",
                "vendor/raylib/src/rcore.c",
                "vendor/raylib/src/rshapes.c",
                "vendor/raylib/src/rtext.c",
                "vendor/raylib/src/rtextures.c",
                "vendor/raylib/src/utils.c",
            }, &.{
                "-std=gnu99",
                "-D_GNU_SOURCE",
                "-DPLATFORM_DESKTOP",
                "-DGRAPHICS_API_OPENGL_33",
                "-fno-sanitize=undefined",
            });

            exe.linkSystemLibrary("m");
            exe.linkSystemLibrary("dl");
            exe.linkSystemLibrary("rt");
            exe.linkSystemLibrary("glfw");
        },
        else => {
            @panic("Unsupported OS");
        },
    }

    exe.install();

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
