# RISC-V-virtual-machine
This is my solution to the [Final assignment] of course [02155 Computer Architecture and Engineering]. Where a simple RISC-V simulation is built for the RV32I Base Integer Instructions. This simulator runs on binary files made in [Ripes].

## risc_v_vm.c
The RISC-V virtual machine in C.
```
risc_v_vm <task.bin>
```
An input file name can also be hardcoded into the binary by uncommenting the input file section in main.
```c
 else{
    file = fopen("test_cases.bin", "rb"); //binary from Ripes
 }
```

The simulator has four different debugging options.
```c
int debug_ins = 0;
int debug_regs = 0;
int debug_memory = 0;
int debug_branch = 0;
```

- debug_ins: Show instructions.  
- debug_regs: Show register values.  
- debug_memory: Show memory content after each write/store instruction.  
- debug_branch: Show branch instruction info.
### Output
The register values are stored in the output file vm_out.res.

## perform_tests.c
To speed up the development process I also built a program to perform the tests.  
Simply place all the *.res and *.bin files from each task in the same folder as the RISC-V simulator and perform_tests file and run the command:
```
perform_tests <risc-v simulator> 
```
It will compare the output from the RISC-V simulator with the expected output given in the *.res file.


## Build

This project was developed and tested on **Linux (Ubuntu 24.04.2 LTS, gcc (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0)**.  
To compile the simulator, use:

```bash
gcc risc_v_vm.c -o risc_v_vm
gcc perform_tests.c -o perform_tests
```
You can then run the simulator with a Ripes-generated binary:
```bash
risc_v_vm <task.bin>
```


## Summary
This RISC-V virtual machine completes all the tasks given in the exercise.  
Compared to [FENIX] challenge I did earlier, this RISC-V virtual machine was rather straightforward to build.

[Final assignment]: https://github.com/schoeberl/cae-lab/tree/master/finasgmt  
[02155 Computer Architecture and Engineering]: https://kurser.dtu.dk/course/02155  
[Ripes]: https://github.com/mortbopet/Ripes  
[FENIX]: https://github.com/Troelsbk/DDIS-Hacker-Academy-Challenge-2019
