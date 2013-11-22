#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define MAXLEN 2560
//    unsigned char dest_buf[0xfffe];

//读取文件filename的内容到dest数组，最多可以读maxlen个字节
//成功返回文件的字节数，失败返回－1

//把用字符表示的十六进制数转换为二进制表示
unsigned char fun(char c)
{
    if('A' <=c && c <='F')
                return c - 'A' + 10;
     if('a' <=c && c <='f')
                return c - 'a' + 10;

    return c - '0' ;
}

//将长度为srclen的char类型数组转化unsigned char类型
void ToHex(const char *src,int srclen,unsigned char *hex )
{
//    unsigned char hex[srclen/2];
    for(unsigned i = 0; i<srclen; ++i )
        if(i%2 == 0){
            hex[i/2] = fun(src[i]);
            hex[i/2]  <<= 4;
        }
        else
            hex[i/2] |= fun(src[i]);
}

int read_file(const char *filename,unsigned char *dest_byte, int maxlen)
{
    FILE *file;
    int pos, i;
    unsigned char temp;
    char *dest;
    dest = ( char *)malloc(sizeof(unsigned char)*maxlen*2);
//    打开文件
    file = fopen(filename, "r");
    if( NULL == file ){
        fprintf(stderr, "open %s error\n", filename);
        return -1;
    }
    pos = 0;
    //循环读取文件中的内容
    for(i=0; i<maxlen; i++){
        temp = fgetc(file);
        if( 0x0a == temp )
            break;
        if( 0x00 == temp )
            break;
        dest[pos++] = temp;
    }
    //    在数组末尾加\0
    dest[pos] = '\0';
    int j=strlen(dest);
    ToHex(dest,j,dest_byte);
//    关闭文件
    fclose(file);
    free(dest);
//    free(dest_byte);
    return j/2;
}
