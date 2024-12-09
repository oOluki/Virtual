# Virtual
Virtual consists of multiple different virtual machines, each with its own paradigms and inner workings, all implementations are independend of each other so you can choose which do one you want.

Virtual Processing Unit:

    Virtual Processing Unit (VPU) is designed to be a simple virtual machine compatiple with multiple platforms. This virtual machine has both registers and a stack and it's inspired by actual CPU assemblies (to some extent) such as X86_64.

Virtual Fiction Emulator:

    Virtual Fiction Emulator (VFE) is designed to be a simple virtual machine compatiple with multiple platforms.
    This virtual machine strides to be very stack based, where most operation take their arguments from the stack, even for operations such as jump (jmp) which generaly don't.
    This virtual machine was highly motivated/influenced by https://github.com/tsoding/bm