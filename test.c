#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>

typedef  union test
{
    uint32_t c;
    uint8_t array[4];

}un;


int main(){
    uint32_t a=0x00011011;
    uint8_t * b=(uint8_t *)(&a);
    un data;
    data.c=0x00011011;

    for(int i=0;i<4;i++){
        printf("%u\n",*(b+i));
    }

    for(int i=0;i<4;i++){
        printf("%u\n",data.array[i]);
    }
    return 0;
}