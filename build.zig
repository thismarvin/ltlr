const std = @import("std");

fn getCSourceFiles(b: *std.Build) !std.ArrayList([]const u8) {
    const directory = "src";

    const absolute_path = try b.build_root.join(b.allocator, &.{directory});

    var iterable_directory = try std.fs.openDirAbsolute(absolute_path, .{ .iterate = true });

    var walker = try iterable_directory.walk(b.allocator);
    defer walker.deinit();

    var source_files_paths = std.ArrayList([]const u8).init(b.allocator);

    while (try walker.next()) |item| {
        if (item.kind != .file) {
            continue;
        }

        const extension = std.fs.path.extension(item.basename);

        if (!std.mem.eql(u8, ".c", extension)) {
            continue;
        }

        const partial = try b.allocator.dupe(u8, item.path);
        const path = try std.fs.path.join(b.allocator, &.{ directory, partial });

        try source_files_paths.append(path);
    }

    return source_files_paths;
}

pub fn build(b: *std.Build) !void {
    var sources = try getCSourceFiles(b);
    defer sources.deinit();

    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "ltlr",
        .target = target,
        .optimize = optimize,
    });

    exe.linkLibC();

    exe.addCSourceFiles(.{ .files = sources.items, .flags = &.{
        "-std=gnu17",
        "-DPLATFORM_DESKTOP",
        "-Ivendor/raylib/src",
        "-Ivendor/wyhash",
    } });

    switch (target.result.os.tag) {
        .windows => {
            exe.addCSourceFiles(.{ .files = &.{
                "vendor/raylib/src/raudio.c",
                "vendor/raylib/src/rcore.c",
                "vendor/raylib/src/rglfw.c",
                "vendor/raylib/src/rshapes.c",
                "vendor/raylib/src/rtext.c",
                "vendor/raylib/src/rtextures.c",
                "vendor/raylib/src/utils.c",
            }, .flags = &.{
                "-std=gnu99",
                "-D_GNU_SOURCE",
                "-DPLATFORM_DESKTOP",
                "-DGRAPHICS_API_OPENGL_33",
                "-Ivendor/raylib/src/external/glfw/include",
                "-Ivendor/raylib/src/external/glfw/deps/mingw",
                "-fno-sanitize=undefined",
            } });

            exe.linkSystemLibrary("opengl32");
            exe.linkSystemLibrary("gdi32");
            exe.linkSystemLibrary("winmm");
        },
        .linux => {
            exe.addCSourceFiles(.{ .files = &.{
                "vendor/raylib/src/raudio.c",
                "vendor/raylib/src/rcore.c",
                "vendor/raylib/src/rshapes.c",
                "vendor/raylib/src/rtext.c",
                "vendor/raylib/src/rtextures.c",
                "vendor/raylib/src/utils.c",
            }, .flags = &.{
                "-std=gnu99",
                "-D_GNU_SOURCE",
                "-DPLATFORM_DESKTOP",
                "-DGRAPHICS_API_OPENGL_33",
                "-fno-sanitize=undefined",
            } });

            exe.linkSystemLibrary("m");
            exe.linkSystemLibrary("dl");
            exe.linkSystemLibrary("rt");
            exe.linkSystemLibrary("glfw");
        },
        else => {
            @panic("Unsupported OS");
        },
    }

    b.installArtifact(exe);

    const run_exe = b.addRunArtifact(exe);

    const run_step = b.step("run", "Run the application");
    run_step.dependOn(&run_exe.step);
}
