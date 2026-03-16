const std = @import("std");

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    const args = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    if (args.len != 2) {
        std.debug.print("usage: bin2c <input>\n", .{});
        std.process.exit(1);
    }

    const data = try std.fs.cwd().readFileAlloc(
        args[1],
        allocator,
        std.Io.Limit.limited(100 * 1024 * 1024),
    );
    defer allocator.free(data);

    var stdout_buf: [65536]u8 = undefined;
    var stdout_writer = std.fs.File.stdout().writerStreaming(&stdout_buf);
    const stdout = &stdout_writer.interface;

    try stdout.writeAll("unsigned char game_main_mrb[] = {\n");
    for (data, 0..) |byte, i| {
        if (i % 12 == 0) try stdout.writeAll("  ");
        try stdout.print("0x{x:0>2}", .{byte});
        if (i < data.len - 1) try stdout.writeAll(", ");
        if ((i + 1) % 12 == 0) try stdout.writeAll("\n");
    }
    try stdout.print(
        "\n}};\nunsigned int game_main_mrb_len = {};\n",
        .{data.len},
    );

    try stdout.flush();
}
