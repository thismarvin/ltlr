const std = @import("std");

pub fn build(b: *std.build.Builder) void {
    const target = b.standardTargetOptions(.{});
    const mode = b.standardReleaseOptions();

    const exe = b.addExecutable("ltlr", null);

    exe.setTarget(target);
    exe.setBuildMode(mode);

    exe.linkLibC();

    exe.addCSourceFiles(&.{
        @panic("For now, this value is auto-generated."),
    }, &.{
        "-std=c17",
        "-DPLATFORM_DESKTOP",
        "-Ivendor/raylib/src",
        "-Ivendor/wyhash",
    });

    switch (target.getOsTag()) {
        .windows => {
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
                "-D_DEFAULT_SOURCE",
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
                "-D_DEFAULT_SOURCE",
                "-DPLATFORM_DESKTOP",
                "-DGRAPHICS_API_OPENGL_33",
                "-fno-sanitize=undefined",
            });

            exe.linkSystemLibrary("m");
            exe.linkSystemLibrary("rt");
            exe.linkSystemLibrary("dl");
            exe.linkSystemLibrary("Xau");
            exe.linkSystemLibrary("Xdmcp");
            exe.linkSystemLibrary("xcb");
            exe.linkSystemLibrary("Xext");
            exe.linkSystemLibrary("Xrender");
            exe.linkSystemLibrary("Xfixes");
            exe.linkSystemLibrary("X11");
            exe.linkSystemLibrary("Xrandr");
            exe.linkSystemLibrary("Xinerama");
            exe.linkSystemLibrary("Xcursor");
            exe.linkSystemLibrary("Xi");
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
