# Debugging

This document explains how to debug programs running in the [mGBA](https://mgba.io/) emulator. It assumes that you are on Linux, adapt commands as needed for
anything else.

## Building Programs For Debugging

For debugging, rebuild your program with the `-g` compiler flag added, so that
debug information is included. You will have a better experience if you also
choose a lower optimisation level such as `-O0`.

## Running mGBA for Debugging

Note: mGBA has had various fixes for debugger compatibilty over time. If you
have problems, use the [latest successful Github
build](https://mgba.io/downloads.html#development-downloads) from buildbot
(`About mGBA` shows the commit hash), or build it from source.

For all the following steps you will be running mGBA either manually or in the
background automatically. This is how you generally do that:

```
mgba --gdb test.gba
```

Note that:
* This command assumes `mgba` is on your `PATH`.
* The default gdb port number is `2345` and this document assumes you have not
  changed this. Adjust as needed if you do.
* You must provide the `.gba` file to mGBA, not the `.elf` file. The `.elf`
  will be given to the debugger later.

## Running a Debugger Manually

Once you have run the above command, you'll run a debugger to connect to the
port that mGBA is listening on.

If you are using lldb (which is included with this toolchain) you need to run:
```
$ $GBA_LLVM/bin/lldb test.elf -o "gdb-remote 2345"
```

Mac OS's system lldb, or one installed from apt will also likely work. The key
thing is it must have been built with the Arm target included. If it doesn't,
it'll likely crash.

For GDB you will need `gdb-multiarch` (available in `apt`), or a GDB from an
Arm cross compilation toolchain.

```
$ gdb-multiarch test.elf
(gdb) target remote :2345
```

Note that in both cases we are giving the debugger the `.elf` file, not the
`.gba` file.

In the debugger you will then see the emulated CPU stopped at the `0x00000000`
reset point. From here you can set breakpoints and continue and do the usual
things you would do when debugging a native program.

When you exit the debugger, mGBA will continue running so you must stop the mGBA
process manually if needed.

## Debugging Using Visual Studio Code

Note: The information here can likely be adapted to any IDE that supports the
[Debug Adapter Protocol (DAP)](https://microsoft.github.io/debug-adapter-protocol//),
but this has only been tested with Visual Studio Code at this time.

Within VS Code you have two options for using LLDB. Using GDB has not been
tested.

If you simply want to debug GBA programs, and may want to to debug native
programs as well, install the
[CodeLLDB](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb)
plugin.

This plugin comes with an LLDB build that supports many architectures including
Arm and is the option most people should choose.

If you are developing the toolchain itself and making changes to LLDB, you can
manually install the `lldb-dap` plugin by following the instructions
[here](https://github.com/llvm/llvm-project/tree/main/lldb/tools/lldb-dap#installation-for-visual-studio-code). Using the `lldb-dap` binary
from this toolchain.

I will talk about both plugins here as they share a lot of the same
configuration in VS Code.

The basic setup is this:
* A task to start mGBA in the background.
* A launch configuration which waits for that task, then connects to mGBA using
  the plugin.
* A task to stop mGBA when VS Code stops the debug session.

This is the content of `tasks.json`:
```
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "run-mgba",
      "type": "shell",
      "isBackground": true,
      "presentation": {"reveal": "never"},
      "problemMatcher": {
        "owner": "run-mgba",
        "pattern": [
          {
            "regexp": "_this_is_required_but_unused_",
            "file": 1,
            "location": 2,
            "message": 3,
          },
        ],
        "background": {
            "activeOnStart": true,
            "beginsPattern":{"regexp": "Debugger: >"},
            "endsPattern":{"regexp": "Debugger: >"}
        },
      },
      "command": "mgba",
      "args": ["--gdb", "${workspaceFolder}/test.gba"]
    },
    {
      "label": "stop-mgba",
      "type": "shell",
      "isBackground": true,
      "presentation": {"reveal": "never"},
      "command": "killall",
      "args": ["mgba"]
    },
  ]
}
```
This content will be the same regardless of plugin being used.

This is the content of `launch.json`:
```
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "mgba-codelldb",
      "type": "lldb",
      "request": "custom",
      "preLaunchTask": "run-mgba",
      "targetCreateCommands": ["target create ${workspaceFolder}/test.elf"],
      "processCreateCommands": ["gdb-remote 2345"],
      "postDebugTask": "stop-mgba"
    },
    {
      "name": "mgba-lldb-dap",
      "type": "lldb-dap",
      "request": "attach",
      "program": "${workspaceFolder}/test.elf",
      "preLaunchTask": "run-mgba",
      "stopOnEntry": true,
      "attachCommands": ["gdb-remote 2345"],
      "postDebugTask": "stop-mgba"
    }
  ]
}
```

Remove whichever plugin you chose not to install. If you have both, you can leave this as is.

With those files setup, you can debug as you would any other program using "Run"
-> "Start Debugging".

Depending on the plugin used, you may not see the current PC location at first.
Usually it will open a disassembly window showing the entry point but not
always.

Place breakpoints in your source files and continue as normal. When they are hit
you will see that in the editor as expected.

If you need to interrupt a running program, the pause button works as you would
expect. It is reported as a `SIGINT` but this is not a bad thing, it's just the
name given to the interrupt signal you the user just sent.

# References

* [CodeLLDB user manual](https://github.com/vadimcn/codelldb/blob/master/MANUAL.md)
* [lldb-dap](https://github.com/llvm/llvm-project/tree/main/lldb/tools/lldb-dap)
