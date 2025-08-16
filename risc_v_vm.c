# define _POSIX_C_SOURCE 200809L

# include <stdio.h>
# include <stdlib.h>
# include <stdint.h>
# include <string.h>
# include <stdbool.h>
# include <sys/stat.h>

//reference card
// https://www.cs.sfu.ca/~ashriram/Courses/CS295/assets/notebooks/RISCV/RISCV_CARD.pdf

# define PC_REG 32
# define REG_ZERO 0

# define mem_size (1<<20)
# define REG(x) vm->registers[x]
# define PC vm.registers[32]

# define MASK_3_BIT 0x07 // funct3
# define MASK_5_BIT 0x1F // register bits
# define MASK_7_BIT 0x7F // opcode bits and funct7

int debug_ins = 0;
int debug_regs = 0;
int debug_memory = 0;
int debug_branch = 0;

# define DEBUG(...) do{ if(debug_ins){ fprintf(stderr, "%#04x ", vm->registers[PC_REG]); fprintf(stderr, __VA_ARGS__);}}while(0)
# define DEBUG_BRANCH(...) do{ if(debug_branch){  fprintf(stderr, __VA_ARGS__);}}while(0)
# define DEBUG_REG(...) do{ if(debug_regs){print_registers(__VA_ARGS__);}  }while(0)
# define DEBUG_MEM(...) do{ if(debug_memory) {print_mem(__VA_ARGS__);} }while(0)

typedef struct instruction_t{
    uint32_t machinecode;
    uint16_t opcode;
    uint16_t rs1;
    uint16_t rs2;
    uint16_t rd;
    uint16_t funct3;
    uint16_t funct7;
    uint16_t f7_index; //for array indexing
    int32_t imm;
    char *name;
}instruction_t;

typedef struct vm_t{
    uint8_t *disk;
    bool running;
    uint32_t registers[33]; //one additinal reg for PC
    uint8_t memory[mem_size];
    bool branch; // true when next PC != PC + 4
}vm_t;

typedef int(*i_opcodes)(vm_t *vm, instruction_t *ins);
typedef int (*r_opcodes)(vm_t *vm, instruction_t *ins);
typedef int(* U_instruction)(vm_t *, instruction_t*);
typedef int(*branch_operations)(vm_t *, instruction_t *);
typedef int(*load_operations)(vm_t*, instruction_t*);
typedef int (*s_type_ins)(vm_t*, instruction_t*);

//debug functions
void print_mem(vm_t *vm, int address, int size);
void print_registers(vm_t *vm);
//decoding and running the virtual machine
void decode(instruction_t *ins);
void run(uint8_t *disk, long signed size);
//instruction implementation
int add(vm_t *vm, instruction_t *ins);
int sub(vm_t *vm, instruction_t *ins);
int xor(vm_t *vm, instruction_t *ins);
int or(vm_t *vm, instruction_t *ins);
int and(vm_t *vm, instruction_t *ins);
int sll(vm_t *vm, instruction_t *ins);
int srl(vm_t *vm, instruction_t *ins);
int sra(vm_t *vm, instruction_t *ins);
int slt(vm_t *vm, instruction_t *ins);
int sltu(vm_t *vm, instruction_t *ins);
int sb(vm_t *vm, instruction_t *ins);
int sh(vm_t *vm, instruction_t *ins);
int sw(vm_t *vm, instruction_t *ins);
int ecall(vm_t *vm);
int jalr(vm_t *vm, instruction_t *ins);
int jal(vm_t *vm, instruction_t *ins);
int addi(vm_t *vm, instruction_t *ins);
int xori(vm_t *vm, instruction_t *ins);
int ori(vm_t *vm, instruction_t *ins);
int andi(vm_t *vm, instruction_t *ins);
int slli(vm_t *vm, instruction_t *ins);
int srli(vm_t *vm, instruction_t *ins);
int srai(vm_t *vm, instruction_t *ins);
int slti(vm_t *vm, instruction_t *ins);
int sltiu(vm_t *vm, instruction_t *ins);
int lui(vm_t *vm, instruction_t *ins);
int auipc(vm_t *vm, instruction_t *ins);
int lb(vm_t *vm, instruction_t *ins);
int lh(vm_t *vm, instruction_t *ins);
int lw(vm_t *vm, instruction_t *ins);
int lbu(vm_t *vm, instruction_t *ins);
int lhu(vm_t *vm, instruction_t *ins);
int beq(vm_t *vm, instruction_t *ins);
int bne(vm_t *vm, instruction_t *ins);
int blt(vm_t *vm, instruction_t *ins);
int bge(vm_t *vm, instruction_t *ins);
int bltu(vm_t *vm, instruction_t *ins);
int bgeu(vm_t *vm, instruction_t *ins);

i_opcodes I_functions_bitwise[8][2] = {
    {addi, NULL},
    {slli, NULL},
    {slti, NULL},
    {sltiu, NULL}, 
    {xori, NULL},
    {srli, srai},
    {ori, NULL}, 
    {andi, NULL},
};

r_opcodes R_functions[8][2] = {
    {add, sub },
    {sll, NULL },
    {slt, NULL },
    {sltu, NULL },
    {xor, NULL },
    {srl, sra },
    {or, NULL},
    {and, NULL }
};

U_instruction U_functions[2] = {lui, auipc};
branch_operations B_functions[] = {beq, bne, NULL, NULL, blt, bge, bltu, bgeu};
load_operations L_functions[] = {lb, lh, lw, NULL, lbu, lhu};
s_type_ins S_functions[] = {sb, sh, sw};

int main(int argc, char *argv[]){

    int fd;
    struct stat st;
    uint8_t *disk;
    FILE *file;
    
    if(argc == 1){
        fprintf(stderr, "Usage: %s <binary input file>\n", argv[0]);
        exit(1);
    }
    else if(argc == 2){
        file = fopen(argv[1], "rb");
    }
    //or hardcode the inputfile into the binary
    //else{
    //    file = fopen("test_cases.bin", "rb"); //binary from Ripes
    //}

    if(file == NULL){
        perror("file open error");
        exit(1);
    }
    else if((fd = fileno(file)) == -1){
        perror("file descriptor error");
        exit(1);
    }
    else if((fstat(fd, &st)) != 0){
        perror("File stat reading error");
        exit(1);
    }
    else if((disk = malloc(st.st_size)) == NULL){
        perror("Malloc error");
        exit(1);
    }
    else if ((fread(disk, 1, st.st_size, file) != (size_t)st.st_size)){
        perror("File read error");
        exit(1);
    }
    else{
        fclose(file);
        run(disk, st.st_size);
    }

free(disk);
return 0;
}

void decode(instruction_t *ins){

    ins->opcode = (ins->machinecode & MASK_7_BIT);
    if(ins->opcode == 0x33){ // type R
        ins->rd = (ins->machinecode >> 7) & MASK_5_BIT;
        ins->rs1 = (ins->machinecode >> 15) & MASK_5_BIT;
        ins->rs2 = (ins->machinecode >> 20) & MASK_5_BIT;
        ins->funct3 = (ins->machinecode >> 12) & MASK_3_BIT;
        ins->funct7 = (ins->machinecode >> 25) & MASK_7_BIT;
        ins->f7_index = (ins->funct7 == 0x20) ? 1 : 0;
    }
    else if(ins->opcode == 0x13 || ins->opcode == 0x67 || ins->opcode == 0x73){ // type I 
        ins->rd = (ins->machinecode >> 7) & MASK_5_BIT;
        ins->funct3 = (ins->machinecode >> 12) & MASK_3_BIT;
        ins->rs1 = (ins->machinecode >> 15) & MASK_5_BIT;
        ins->imm = (ins->machinecode >> 20);
        if((ins->imm & 0x800) == 0x800){ //sign extend if negative
            ins->imm |= 0xfffff000;
        }
        ins->funct7 = (ins->machinecode >> 25) & MASK_7_BIT;
        ins->f7_index = (ins->funct7 == 0x20) ? 1: 0;
    }
    else if(ins->opcode == 0x37 || ins->opcode == 0x17){ // lui or auipc
        ins->rd = (ins->machinecode >> 7) & MASK_5_BIT;
        ins->imm = (ins->machinecode >> 12);
    }
    else if(ins->opcode == 0x23){ //store operation
        ins->rs1 = (ins->machinecode >> 15) & MASK_5_BIT;
        ins->rs2 = (ins->machinecode >> 20) & MASK_5_BIT;
        ins->funct3 = (ins->machinecode >> 12) & MASK_3_BIT;
        uint32_t imm_low = (ins->machinecode >> 7) & MASK_5_BIT;
        uint32_t imm_high = (ins->machinecode >> 25) << 5;
        ins->imm = imm_high | imm_low;
        if((ins->imm & 0x800) == 0x800){ //sign extend if negative
            ins->imm |= 0xfffff000;
        }
    }
    else if(ins->opcode == 0x03){ //load operation
        //same as I type
        ins->rd = (ins->machinecode >> 7) & MASK_5_BIT;
        ins->funct3 = (ins->machinecode >> 12) & MASK_3_BIT;
        ins->rs1 = (ins->machinecode >> 15) & MASK_5_BIT;
        ins->imm = (ins->machinecode >> 20);
        if((ins->imm & 0x800) == 0x800){ //sign extend if negative
            ins->imm |= 0xfffff000;
        }
        ins->funct7 = (ins->machinecode >> 25) & MASK_7_BIT;
    }
    else if(ins->opcode == 0x63){ //branch
        ins->rs1 = (ins->machinecode >> 15) & MASK_5_BIT;
        ins->rs2 = (ins->machinecode >> 20) & MASK_5_BIT;
        ins->funct3 = (ins->machinecode >> 12) & MASK_3_BIT;
        uint32_t imm12 = (ins->machinecode >> 31) & 0x1;
        uint32_t imm10_5 = (ins->machinecode >> 25) & 0x3F;
        uint32_t imm4_1 = (ins->machinecode >> 8) & 0xF;
        uint32_t imm11 = (ins->machinecode >> 7) & 0x1; 
        ins->imm = (imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1);
        ins->imm = (int32_t)(ins->imm << (32 - 13)) >> (32 - 13); //imm is 13 bits long
    }
    else if(ins->opcode == 0x6F){ //jal
        ins->rd = (ins->machinecode >> 7) & MASK_5_BIT;
        uint32_t imm20   = (ins->machinecode >> 31) & 0x1;
        uint32_t imm10_1 = (ins->machinecode >> 21) & 0x3FF;
        uint32_t imm11   = (ins->machinecode >> 20) & 0x1;
        uint32_t imm19_12 = (ins->machinecode >> 12) & 0xFF;
        ins->imm =     (imm20 << 20) | (imm19_12 << 12) |
                       (imm11 << 11) | (imm10_1 << 1);
        // sign extend
        ins->imm = ((int32_t)(ins->imm << (32 - 20))) >> (32-20);
    }
    else{
        fprintf(stderr, "Decode: Unknown opcode %#x\n", ins->opcode);
        exit(1);
    }
}

void run(uint8_t *disk, long signed size){

    vm_t vm;
    memset(&vm, 0x00, sizeof(vm)); // set registers to zero
    vm.disk = disk;
    memcpy(vm.memory, vm.disk, size);// copy data from disk to mem.
    vm.branch = false;
    vm.running = true; //turn on machine

    instruction_t instruction;

    while(vm.running){

        instruction.machinecode = (vm.memory[PC + 0]) <<  0 |
                                  (vm.memory[PC + 1]) <<  8 |
                                  (vm.memory[PC + 2]) << 16 |
                                  (vm.memory[PC + 3]) << 24;

        decode(&instruction);
        vm.registers[REG_ZERO] = 0;
        if(instruction.opcode == 0x33){ //R-type
            R_functions[instruction.funct3][instruction.f7_index](&vm, &instruction);
        }
        else if(instruction.opcode == 0x13){ // I-type
            I_functions_bitwise[instruction.funct3][instruction.f7_index](&vm, &instruction);
        }
        else if(instruction.opcode == 0x37){
            lui(&vm, &instruction);
        }
        else if(instruction.opcode == 0x17){
            auipc(&vm, &instruction);
        }
        else if(instruction.opcode == 0x23){
            S_functions[instruction.funct3](&vm, &instruction);
        }
        else if(instruction.opcode == 0x03){
            L_functions[instruction.funct3](&vm, &instruction);
        }
        else if(instruction.opcode == 0x63){
            B_functions[instruction.funct3](&vm, &instruction);
        }
        else if(instruction.opcode == 0x6F){
            jal(&vm, &instruction);
        }
        else if(instruction.opcode == 0x67){
            jalr(&vm, &instruction);
        }
        else if(instruction.opcode == 0x73){ //ecall
            ecall(&vm);
        }
        else{
            fprintf(stderr, "Unknown instruction: Opcode=%#x\n", instruction.opcode);
            exit(100);
        }

        if(!vm.branch){
            vm.registers[PC_REG] += 4;
        }
        else{
            vm.branch = false;
        }

        if(vm.registers[PC_REG] % 4 != 0){
            fprintf(stderr, "Memory alignment error.");
            exit(1);
        }
    }
}

int sb(vm_t *vm, instruction_t *ins){
    vm->memory[REG(ins->rs1) + ins->imm] = (REG(ins->rs2) & 0xFF);
    DEBUG("SB x%i imm=%i %#x\n", ins->rs2, (ins->imm & 0xFF), ins->rs1);
    DEBUG_REG(vm);
    DEBUG_MEM(vm, REG(ins->rs1) + ins->imm, 4);
    return 0;
}

int sh(vm_t *vm, instruction_t *ins){
    vm->memory[REG(ins->rs1) + ins->imm] = (REG(ins->rs2) & 0xFF);
    vm->memory[(REG(ins->rs1) + ins->imm) + 1] = ((REG(ins->rs2) & 0xFF00) >> 0x8);
    DEBUG("SH x%i imm=%i %#x\n", ins->rs2, (ins->imm & 0xFFFF), ins->rs1);
    DEBUG_REG(vm);
    DEBUG_MEM(vm, REG(ins->rs1) + ins->imm, 4);
    return 0;
}

int sw(vm_t *vm, instruction_t *ins){
    *(uint32_t*)(&vm->memory[REG(ins->rs1) + ins->imm]) = REG(ins->rs2);
    DEBUG("SW x%i imm=%i %#x\n", ins->rs2, ins->imm, ins->rs1);
    DEBUG_REG(vm);
    DEBUG_MEM(vm, REG(ins->rs1) + ins->imm, 4);
    return 0;
}

int add(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = REG(ins->rs1) + REG(ins->rs2);
    DEBUG("ADD x%i x%i x%i\n", ins->rd, ins->rs1, ins->rs2);
    DEBUG_REG(vm);
    return 0;
}

int sub(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = REG(ins->rs1) - REG(ins->rs2);
    DEBUG("SUB x%i x%i x%i\n", ins->rd, ins->rs1, ins->rs2);
    DEBUG_REG(vm);
    return 0;
}

int xor(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = REG(ins->rs1) ^ REG(ins->rs2);
    DEBUG("XOR x%i x%i x%i\n", ins->rd, ins->rs1, ins->rs2);
    DEBUG_REG(vm);
    return 0;
}

int or(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = REG(ins->rs1) | REG(ins->rs2);
    DEBUG("OR x%i x%i x%i\n", ins->rd, ins->rs1, ins->rs2);
    DEBUG_REG(vm);
    return 0;
}

int and(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = REG(ins->rs1) & REG(ins->rs2);
    DEBUG("AND x%i x%i x%i\n", ins->rd, ins->rs1, ins->rs2);
    DEBUG_REG(vm);
    return 0;
}

int sll(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = REG(ins->rs1) << REG(ins->rs2);
    DEBUG("SLL x%i x%i x%i\n", ins->rd, ins->rs1, ins->rs2);
    DEBUG_REG(vm);
    return 0;
}

int srl(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = REG(ins->rs1) >> REG(ins->rs2);
    DEBUG("SRL x%i x%i x%i\n", ins->rd, ins->rs1, ins->rs2);
    DEBUG_REG(vm);
    return 0;
}

int sra(vm_t *vm, instruction_t *ins){
    if(REG(ins->rs1) & 0x80000000){
        REG(ins->rd) = (int32_t) REG(ins->rs1) >> REG(ins->rs2);
    }
    else{
        REG(ins->rd) = REG(ins->rs1) >> REG(ins->rs2);
    }
    DEBUG("SRA x%i x%i x%i\n", ins->rd, ins->rs1, ins->rs2);
    DEBUG_REG(vm);
    return 0;
}

int slt(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = ((int32_t)REG(ins->rs1) < (int32_t)REG(ins->rs2)) ? 1:0; 
    DEBUG("SLT x%i x%i x%i\n", ins->rd, ins->rs1, ins->rs2);
    DEBUG_REG(vm);
    return 0;
}

int sltu(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = ((uint32_t)REG(ins->rs1) < (uint32_t)REG(ins->rs2)) ? 1:0; 
    DEBUG("SLTU x%i x%i x%i\n", ins->rd, ins->rs1, ins->rs2);
    DEBUG_REG(vm);
    return 0;
}

void print_registers(vm_t *vm){
    fprintf(stderr, "Registers:\n");
    for (int index = 0; index < 33; index++) {
        fprintf(stderr, "x%-2d = 0x%08x  ", index, vm->registers[index]);
        if ((index + 1) % 4 == 0 || index == 32) {
            fprintf(stderr, "\n");
        }
    }
fprintf(stderr, "\n");
}

void print_mem(vm_t *vm, int address, int size){
    fprintf(stderr, "Memory:");
    
    for(int index = 0; index < size; index++){
       if(index % 4 == 0){
           fprintf(stderr, "\n%#06x  ", address + index);
       }
    fprintf(stderr, "0x%02x  ", vm->memory[address + index]);

    }
    fprintf(stderr, "\n\n");
}

int ecall(vm_t *vm){
    DEBUG_REG(vm);
    if(REG(17) == 10 || REG(10) == 10){
        vm->running = false;

        FILE *fp = fopen("vm_out.res", "wb");
        if(fp == NULL){
            fprintf(stderr, "ecall: file open error\n");
            exit(1);
        }
        size_t write_out = fwrite(vm->registers, sizeof(uint32_t), 32, fp);
        if(write_out != 32){
            fprintf(stderr, "ecall: file write error. write_out=%zu\n", write_out);
            exit(1);
            fclose(fp);
        }

        fclose(fp);

        if(debug_ins){
            puts("ecall: Register value saved in reg_out.bin");
        }
    }
    return 0;
}
int jalr(vm_t *vm, instruction_t *ins){
    uint32_t current_PC = REG(PC_REG);
    REG(ins->rd) = REG(PC_REG) + 4;
    REG(PC_REG) = ins->imm + REG(ins->rs1);
    vm->branch = true;
    DEBUG_BRANCH("%#04x JALR x%i %i\n", current_PC, ins->rd, ins->imm);
    DEBUG_REG(vm);
    return 0;
}

int jal(vm_t *vm, instruction_t *ins){
    uint32_t current_PC = REG(PC_REG);
    REG(ins->rd) = REG(PC_REG) + 4;
    REG(PC_REG) += ins->imm;
    vm->branch = true;
    DEBUG_BRANCH("%#04x JAL x%i %i\n", current_PC, ins->rd, ins->imm);
    DEBUG_REG(vm);
    return 0;
}

int addi(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = REG(ins->rs1) + ins->imm;
    DEBUG("ADDI  x%i x%i imm=%#x\n", ins->rd, ins->rs1, ins->imm);
    DEBUG_REG(vm);
    return 0;
}

int xori(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = REG(ins->rs1) ^ ins->imm;
    DEBUG("XORI  x%i x%i imm=%#x\n", ins->rd, ins->rs1, ins->imm);
    DEBUG_REG(vm);
    return 0;
}

int ori(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = REG(ins->rs1) | ins->imm;
    DEBUG("ORI    x%i x%i imm=%#x\n", ins->rd, ins->rs1, ins->imm);
    DEBUG_REG(vm);
    return 0;
}

int andi(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = REG(ins->rs1) & ins->imm;
    DEBUG("ANDI  x%i x%i imm=%#x\n", ins->rd, ins->rs1, ins->imm);
    DEBUG_REG(vm);
    return 0;
}

int slli(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = REG(ins->rs1) << (ins->imm & MASK_5_BIT);
    DEBUG("SLLI   x%i x%i imm=%#x\n", ins->rd, ins->rs1, (ins->imm & MASK_5_BIT));
    DEBUG_REG(vm);
    return 0;
}

int srli(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = REG(ins->rs1) >> (ins->imm & MASK_5_BIT);
    DEBUG("SRLI   x%i x%i imm=%#x\n", ins->rd, ins->rs1, (ins->imm & MASK_5_BIT));
    DEBUG_REG(vm);
    return 0;
}

int srai(vm_t *vm, instruction_t *ins){
    if(REG(ins->rs1) & 0x80000000){
        REG(ins->rd) = (int32_t) REG(ins->rs1) >> (ins->imm & MASK_5_BIT);
    }
    else{
        REG(ins->rd) = REG(ins->rs1) >> (ins->imm & MASK_5_BIT);
    }
    DEBUG("SRAI   x%i x%i imm=%#x\n", ins->rd, ins->rs1, (ins->imm & MASK_5_BIT));
    DEBUG_REG(vm);
    return 0;
}

int slti(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = (int32_t)REG(ins->rs1) < (int32_t)ins->imm ? 1:0;
    DEBUG("SLTI   x%i x%i imm=%#x\n", ins->rd, ins->rs1, ins->imm );
    DEBUG_REG(vm);
    return 0;
}

int sltiu(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = (uint32_t)REG(ins->rs1) < (uint32_t)ins->imm ? 1:0;
    DEBUG("SLTI   x%i x%i imm=%#x\n", ins->rd, ins->rs1, ins->imm );
    DEBUG_REG(vm);
    return 0;
}

int lui(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = (ins->imm << 12);
    DEBUG("LUI x%i imm=%#x\n", ins->rd, ins->imm);
    DEBUG_REG(vm);
    return 0;
}

int auipc(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = REG(PC_REG) + (ins->imm << 12);
    DEBUG("AUIPC x%i imm=%#x\n", ins->rd,  ins->imm);
    DEBUG_REG(vm);
    return 0;
}


int lb(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = (int8_t)vm->memory[REG(ins->rs1) + ins->imm];
    DEBUG("LB x%i imm=%i %#x\n", ins->rs2, ins->imm, ins->rs1);
    DEBUG_REG(vm);
    return 0;
}

int lh(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = *(int16_t*)(vm->memory + REG(ins->rs1) + ins->imm);
    DEBUG("LH x%i imm=%i %#x\n", ins->rs2, ins->imm, ins->rs1);
    DEBUG_REG(vm);
    return 0;
}

int lw(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = *(int32_t*)(vm->memory + REG(ins->rs1) + ins->imm);
    DEBUG("LW x%i imm=%i %#x\n", ins->rs2, ins->imm, ins->rs1);
    DEBUG_REG(vm);
    return 0;
}

int lbu(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = (uint8_t)vm->memory[REG(ins->rs1) + ins->imm];
    DEBUG("LBU x%i imm=%i %#x\n", ins->rs2, ins->imm, ins->rs1);
    DEBUG_REG(vm);
    return 0;
}

int lhu(vm_t *vm, instruction_t *ins){
    REG(ins->rd) = *(uint16_t*)(vm->memory + REG(ins->rs1) + ins->imm);
    DEBUG("LHU x%i imm=%i %#x\n", ins->rs2, ins->imm, ins->rs1);
    DEBUG_REG(vm);
    return 0;
}

int beq(vm_t *vm, instruction_t *ins){
    uint32_t current_PC = REG(PC_REG);
    if(REG(ins->rs1) == REG(ins->rs2)){
        REG(PC_REG) += (int32_t)ins->imm;
        vm->branch = true;
    }
    DEBUG_BRANCH("%#04x BEQ x%i x%i %i\n", current_PC, ins->rs1, ins->rs2, (int32_t)ins->imm);
    DEBUG_REG(vm);
    return 0;
}

int bne(vm_t *vm, instruction_t *ins){
    uint32_t current_PC = REG(PC_REG);
    if(REG(ins->rs1) != REG(ins->rs2)){
        REG(PC_REG) += (int32_t)ins->imm;
        vm->branch = true;
    }
    DEBUG_BRANCH("%#04x BNE x%i x%i %i\n", current_PC, ins->rs1, ins->rs2, (int32_t)ins->imm);
    DEBUG_REG(vm);
    return 0;
}

int blt(vm_t *vm, instruction_t *ins){
    uint32_t current_PC = REG(PC_REG);
    if((int32_t)REG(ins->rs1) < (int32_t)REG(ins->rs2)){
        REG(PC_REG) += (int32_t)ins->imm;
        vm->branch = true;
    }
    DEBUG_BRANCH("%#04x BLT x%i x%i %i\n", current_PC, ins->rs1, ins->rs2, (int32_t)ins->imm);
    DEBUG_REG(vm);
    return 0;
}

int bge(vm_t *vm, instruction_t *ins){
    uint32_t current_PC = REG(PC_REG);
    if((int32_t)REG(ins->rs1) >= (int32_t)REG(ins->rs2)){
        REG(PC_REG) += (int32_t)ins->imm;
        vm->branch = true;
    }
    DEBUG_BRANCH("%#04x BGE x%i x%i %i\n", current_PC, ins->rs1, ins->rs2, (int32_t)ins->imm);
    DEBUG_REG(vm);
    return 0;
}


int bltu(vm_t *vm, instruction_t *ins){
    uint32_t current_PC = REG(PC_REG);
    if(REG(ins->rs1) < REG(ins->rs2)){
        REG(PC_REG) += (int32_t)ins->imm;
        vm->branch = true;
    }
    DEBUG_BRANCH("%#04x BLTU x%i x%i %i\n", current_PC, ins->rs1, ins->rs2, (int32_t)ins->imm);
    DEBUG_REG(vm);
    return 0;
}

int bgeu(vm_t *vm, instruction_t *ins){
    uint32_t current_PC = REG(PC_REG);
    if(REG(ins->rs1) >= REG(ins->rs2)){
        REG(PC_REG) += (int32_t)ins->imm;
        vm->branch = true;
    }
    DEBUG_BRANCH("%#04x BGEU x%i x%i %i\n", current_PC, ins->rs1, ins->rs2, (int32_t)ins->imm);
    DEBUG("bgeu x%i x%i %i\n", ins->rs1, ins->rs2, (int32_t)ins->imm);
    DEBUG_REG(vm);
    return 0;
}
