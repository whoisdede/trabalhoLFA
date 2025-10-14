#include "dataset.h"

int main(){
    int largura = 8;
    int tamPalavra = log2(largura);
    int size = largura*largura*tamPalavra + largura*largura +largura+1;

    printf("%d\n",size);
    char palavra[265]= {"222.221.212.211.122.121.112.111.N223.224.213.214.123.124.113.114.N232.231.242.241.132.131.142.141.N233.234.243.244.133.134.143.144.N322.321.312.311.422.421.412.411.N323.324.313.314.423.424.413.414.N332.331.342.341.432.431.442.441.N333.334.343.344.433.434.443.444.N"};
    
    for (int i = 0; i < size; i++)
    {
        if(palavra[i] == 'N') palavra[i] = '\n';
    }
    
    
    printf("%s\n", palavra);
    return 0;
}