#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include <assert.h>
#define ROM_MaxSize (1U<<20)
#define MEM_Maxsize (1U<<20)
uint32_t PC=0;
uint32_t R[32];
uint8_t MEM[MEM_Maxsize];

typedef union minirv_ROM{
    uint8_t M[ROM_MaxSize];
    uint32_t inst[ROM_MaxSize/4];
}minirv_ROM;


//Init the ROM

    /*
        0xfec00513,  // 0x00: addi a0, zero, -20
        0x010000e7,  // 0x04: jalr ra, 16(zero)
        0x00c000e7,  // 0x08: jalr ra, 12(zero)
        0x00c00067,  // 0x0c: jalr zero,12(zero)
        0xff650513,  // 0x10: addi a0,a0,-10
        0x00008067   // 0x14: jalr zero,0(ra)

        0x01400513：000000010100 00000 000 01010 0010011
        0xfec00513：111111101100 00000 000 01010 0010011

        0x00a50513：000000001010 01010 000 01010 0010011
        0xff650513：111111110110 01010 000 01010 0010011

        0x00002083：000000000000 00000 010 00001 0000011    lw 1 0 0
        0x00004103：000000000000 00000 100 00010 0000011    lbu 2 0 0
        0x00104183：000000000001 00000 100 00011 0000011    lbu 3 1 0
        0x00204203：000000000010 00000 100 00100 0000011    lbu 4 2 0
        0x00304283：000000000011 00000 100 00101 0000011    lbu 5 3 0

        0x00a02023:0000000 01010 00000 010 00000 0100011   sw 0 r[10]
        0x00b00023:0000000 01011 00000 000 00000 0100011   sb 0 r[11]
        0x:0000000 01100 00000 000 00001 0100011    sb 1 r[12]
        0000000 01101 00000 000 00010 0100011   sb 2 r[13]
        0000000 01110 00000 000 00011 0100011   sb 3 r[14]

        (funct3==0) && (opcode==0x23)

    */

minirv_ROM ROM={
        .inst={
            0x00a02023, //sw

            0x00002083, //lw

            0x00004103, //lbu
            0x00104183,
            0x00204203,
            0x00304283,

            0x00b00023, //sb
            0b00000000110000000000000010100011,
            0b00000000110100000000000100100011, 
            0b00000000111000000000000110100011,

            0x00002083, //lw

            0x00004103, //lbu
            0x00104183,
            0x00204203,
            0x00304283,

        /* 0xfec00513,  // 0x00: addi a0, zero, -20
        0x010000e7,  // 0x04: jalr ra, 16(zero)
        0x00c000e7,  // 0x08: jalr ra, 12(zero)
        0x00c00067,  // 0x0c: jalr zero,12(zero)
        0xff650513,  // 0x10: addi a0,a0,-10
        0x00008067   // 0x14: jalr zero,0(ra)
        */
    }

};

uint8_t get_7_bit_opcode(uint8_t inst){
    return (inst & 0x7F);
}

uint8_t get_5_bit_rd(uint8_t inst1, uint8_t inst2){
    return (((inst1&0x0F)<<1)|(inst2>>7));
}

uint8_t get_3_bit_funct3(uint8_t inst){
    return ((inst&0x70)>>4);
}

uint8_t get_5_bit_rs1(uint8_t inst1, uint8_t inst2){
    return (((inst1&0x0F)<<1)|(inst2>>7));
}

uint8_t get_5_bit_rs2(uint8_t inst1, uint8_t inst2){
    return (((inst1&0x01)<<4)|(inst2>>4));
}

uint32_t get_12_bit_imm(int8_t inst1, uint8_t inst2){
    return ((((uint32_t)inst1)<<4)|(((uint32_t)inst2)>>4));
}

uint8_t get_7_bit_funct7(uint8_t inst1){
    return (inst1>>1);
}

uint32_t get_32_bit_U_imm(uint8_t inst0, uint8_t inst1, uint8_t inst2){
    return (((uint32_t)inst0<<24) | ((uint32_t)inst1<<16) | ((((uint32_t)inst2)&0xf0)<<8));
}

uint8_t get_7_bit_imm(uint8_t inst1){
    return get_7_bit_funct7(inst1);
}

uint8_t get_5_bit_imm(uint8_t inst1, uint8_t inst2){
    return get_5_bit_rd(inst1,inst2);
}



void reg_write(uint8_t rd, uint32_t val){
    if(rd!=0){
        R[rd]=val;
    }
    return;
}

void mem_write(uint32_t addr, uint8_t val){
    MEM[addr]=val;
}


void test(){
    //取指
    uint8_t inst0=ROM.M[PC+3];
    uint8_t inst1=ROM.M[PC+2];
    uint8_t inst2=ROM.M[PC+1];
    uint8_t inst3=ROM.M[PC+0];

    printf("opcode:%u\n",get_7_bit_opcode(inst3));
    printf("5rd:%u\n",get_5_bit_rd(inst2,inst3));
    printf("3funct:%u\n",get_3_bit_funct3(inst2));
    printf("5rs1:%u\n",get_5_bit_rs1(inst1,inst2));
    printf("5rs2:%u\n",get_5_bit_rs2(inst0,inst1));
    printf("12imm:%d\n",(int)get_12_bit_imm(inst0,inst1));
}


void inst_cycle(){
    //取指
    uint8_t inst0=ROM.M[PC+3];
    uint8_t inst1=ROM.M[PC+2];
    uint8_t inst2=ROM.M[PC+1];
    uint8_t inst3=ROM.M[PC+0];

    //译码

    uint8_t opcode=get_7_bit_opcode(inst3);
    uint8_t funct3=get_3_bit_funct3(inst2);
    uint8_t funct7=get_7_bit_funct7(inst0);

    uint8_t rd=get_5_bit_rd(inst2,inst3);
    uint8_t rs1=get_5_bit_rs1(inst1,inst2);
    uint8_t rs2=get_5_bit_rs2(inst0,inst1);

    uint32_t U_imm=get_32_bit_U_imm(inst0,inst1,inst2);
    
        //ADDI
        if((opcode==0x13) && (funct3==0)){
            //执行

            //译码
            uint32_t imm=get_12_bit_imm(inst0,inst1);


            //立即数符号扩展
            uint32_t se_imm=((imm>>11)&0x1)==1?(imm|0xFFFFF000):imm;

            //加和
            reg_write(rd,R[rs1]+se_imm);

        }

        //JALR
        else if((opcode==0x67) && (funct3==0)){
            //执行
            //译码
            uint32_t imm=get_12_bit_imm(inst0,inst1);

            //立即数符号扩展
            uint32_t se_imm=((imm>>11)&0x1)==1?(imm|0xFFFFF000):imm;

            //存地址
            uint32_t return_addr=PC+4;

            //跳转
            PC=(se_imm+R[rs1])&(0xFFFFFFFE);

            reg_write(rd,return_addr);
            
            return ;
        }

        //ADD
        else if((funct7==0) && (funct3==0) && (opcode==0x33)){
            //执行
            reg_write(rd,R[rs1]+R[rs2]);
        }

        //LUI
        else if(opcode==0x37){
            reg_write(rd,U_imm);
        }

        //LW
        else if((funct3==0x2) && (opcode==0x03)){
            uint32_t imm=get_12_bit_imm(inst0,inst1);
            uint32_t se_imm=((imm>>11)&0x1)==1?(imm|0xFFFFF000):imm;
            uint32_t addr=R[rs1]+se_imm;
            uint32_t Byte0=(uint32_t)(MEM[addr+3]);
            uint32_t Byte1=(uint32_t)(MEM[addr+2]);
            uint32_t Byte2=(uint32_t)(MEM[addr+1]);
            uint32_t Byte3=(uint32_t)(MEM[addr]);
            uint32_t data=(Byte0<<24) | (Byte1<<16) | (Byte2<<8) | Byte3;
            reg_write(rd, data);
        }

        //SW
        else if((funct3==0x2) && (opcode==0x23)){
            uint32_t imm1=((uint32_t)(get_7_bit_imm(inst0)))<<5;
            uint32_t imm2=(uint32_t)(get_5_bit_imm(inst2,inst3));
            uint32_t imm=imm1|imm2;
            uint32_t se_imm=((imm>>11)&0x1)==1?(imm|0xFFFFF000):imm;
            uint32_t addr=R[rs1]+se_imm;
            uint32_t data=R[rs2];
            uint8_t Byte0=(uint8_t)(data>>24);
            uint8_t Byte1=(uint8_t)(data>>16);
            uint8_t Byte2=(uint8_t)(data>>8);
            uint8_t Byte3=(uint8_t)data;
            mem_write(addr,Byte3);
            mem_write(addr+1,Byte2);
            mem_write(addr+2,Byte1);
            mem_write(addr+3,Byte0);
        }
 
        //LBU
        else if((funct3==0x4) && (opcode==0x03)){
            uint32_t imm=get_12_bit_imm(inst0,inst1);
            uint32_t se_imm=((imm>>11)&0x1)==1?(imm|0xFFFFF000):imm;
            uint32_t addr=R[rs1]+se_imm;
            uint32_t data=(uint32_t)(MEM[addr]);
            reg_write(rd,data);
        }

        //SB
        else if((funct3==0) && (opcode==0x23)){
            uint32_t imm1=((uint32_t)(get_7_bit_imm(inst0)))<<5;
            uint32_t imm2=(uint32_t)(get_5_bit_imm(inst2,inst3));
            uint32_t imm=imm1|imm2;
            uint32_t se_imm=((imm>>11)&0x1)==1?(imm|0xFFFFF000):imm;
            uint32_t addr=R[rs1]+se_imm;
            uint32_t data=R[rs2];
            uint8_t Byte=(uint8_t)data;
            mem_write(addr, Byte);
        }

        //更新PC
        PC+=4;

}

void Init(char *FilePath){
    FILE* fp=fopen(FilePath, "rb");
    assert(fp!=NULL);
    int num1=fread(MEM,1,MEM_Maxsize,fp);
    assert(num1<MEM_Maxsize);
    rewind(fp);
    uint32_t num2=fread(ROM.M,1,ROM_MaxSize,fp);
    assert(num2<ROM_MaxSize);
    fclose(fp);
}

int main(){
    Init("./logisim-bin/sum.bin");
    /*
    R[10]=0x12345678;
    R[11]=0xef;
    R[12]=0xcd;
    R[13]=0xab;
    R[14]=0x90;
    */
    int j=7002;
    while(j>0) {
        inst_cycle();
        j--;
    }
    printf("%d\n",PC);
    printf("0x%08x\n",R[10]);
    return 0;
}
