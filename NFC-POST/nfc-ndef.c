#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <stdint.h>
#include <stdbool.h>
#include "nfc-ndef.h"
#define MaxLength 1000

//typedef unsigned char uint8_t;
//对于返回值outputSize，当*pInput为单字节UTF8字符时，outputSize+1.当*pInput为双字节或双字节以上的UTF8字符时，outputSize+2.
/*
int input2ASC_Text(char input[MaxLength], char payload[MaxLength])
{
    int outputSize = 0; //记录转换后的Unicode字符串的字节数
    int outputLen = 0;

    char Output[MaxLength][2] = {};

    int i = 0;
    int temp = 0;//临时变量，用于遍历输出字符串

    while (input[i])
    {
        if (input[i] > 0x00 && input[i] <= 0x7F) //处理单字节UTF8字符（英文字母、数字）
        {
            Output[temp][0] = input[i];
            outputSize += 1;
            outputLen += 1;
        }

        else if (((input[i]) & 0xE0) == 0xC0) //处理双字节UTF8字符
        {
            char high = input[i];
            i++;
            char low = input[i];
            if ((low & 0xC0) != 0x80)  //检查是否为合法的UTF8字符表示
            {
                return -1; //如果不是则报错
            }

            Output[temp][0] = (high >> 2) & 0x07;
            Output[temp][1] = (high << 6) + (low & 0x3F);
            outputSize += 2;
            outputLen += 1;
        }
        else if (((input[i]) & 0xF0) == 0xE0) //处理三字节UTF8字符
        {
            char high= input[i];
            i++;
            char middle = input[i];
            i++;
            char low = input[i];
            if (((middle & 0xC0) != 0x80) || ((low & 0xC0) != 0x80))
            {
                return -1;
            }
            Output[temp][0] = (high << 4) + ((middle >> 2) & 0x0F);
            //printf("0x%x ",*tmp);
            Output[temp][1] = (middle << 6) + (low & 0x7F);
            //printf("0x%x ",*tmp);
            //printf(" ");
            outputSize += 2;
            outputLen += 1;
        }
        else //对于其他字节数的UTF8字符不进行处理
        {
            return -1;
        }
        i ++;
        temp ++;
        //outputSize += 2;
    }
    //printf("%d\n",outputSize);
    //printf("%d\n",outputLen);
    int j = 0;
    int cnt =0;
    for(j =0; j < outputLen; j++)
    {
        if(Output[j][1])
        {
            payload[cnt] = Output[j][0];
            payload[cnt+1] = Output[j][1];
            cnt +=2;
        }
        else
        {
            payload[cnt] = Output[j][0];
            cnt ++;
        }

    }
    return outputSize;
}
*/

/*
该函数的输入为pInput[MaxLength]，它同时也是input2ASC_single函数的输出，该函数输出结果存放
在Record[MaxLength]一维数组之中。offset表示该条tag的位置，参数有三种取值分别是：0，1，2，3
offset =0时，表示只有一条tag记录
offset =1时，表示有多条记录，且该条记录位于整个信息的首部
offset =2时，表示有多条记录，且该条记录位于整个信息的中部
offset =3时，表示有多条记录，且该条记录位于整个信息的尾部
length表示将输入的字符串经过处理之后，得到的16进制ASCII码的长度，它同时也是input2ASC_single函数的返回值。
*/
int ndef_Text(char input[MaxLength],uint8_t Record[MaxLength],int offset)
{
    int recordLength;
    int length = strlen(input);
    if(length <= 255)   //当经过处理后的字符串长度小于等于255时，自动设SR=1，即PAYLOAD_LENGTH字段为1字节。
    {
        if(offset == 0)
        {
            Record[0] = 0xD1;          //MB =1,ME =1.
        }
        else if(offset == 1)
        {
            Record[0] = 0x91;          //MB =1,ME =0.
        }
        else if (offset == 2)
        {
            Record[0] = 0x11;          //MB =0,ME =0.
        }
        else if(offset == 3)
        {
            Record[0] = 0x51;          //MB =0,ME =1.
        }
        else
        {
            return -1;
        }
        Record[1] = 0x01;          // Type Length 为1字节。
        Record[2] = length+3;   //PAYLOAD_LENGTH
        Record[3] = 0x54;          //Type的值为0x54，即“T”
        Record[4] = 0x02;          //language code length ,由于是“en”，所以长度为2字节。
        Record[5] = 0x65;          //e
        Record[6] = 0x6e;          //n

        int j,k;
        for(j=7,k=0; input[k]!='\0'; j++,k++)
        {
            Record[j] = input[k];
        }
        recordLength = length + 7;
    }
    else
    {
        if(offset == 0)
        {
            Record[0] = 0xC1;          //MB=1,ME=1
        }
        else if(offset == 1)
        {
            Record[0] = 0x81;          //MB=1,ME=0
        }
        else if(offset == 2)
        {
            Record[0] = 0x01;          //MB=0,ME=0
        }
        else if(offset == 3)
        {
            Record[0] = 0x41;          //MB=0,ME=1
        }
        else
        {
            return -1;
        }
        Record[1] = 0x01;
        Record[2] = (length/256/256/256)%256;
        Record[3] = (length/256/256)%256;
        Record[4] = (length/256)%256;
        Record[5] = length%256;
        Record[6] = 0x54;
        Record[7] = 0x02;
        Record[8] = 0x65;
        Record[9] = 0x6e;

        int j,k;
        for(j=10,k=0; input[k]!='\0'; j++,k++)
        {
            Record[j] = input[k];
        }
        recordLength = length + 10;
    }
    return recordLength;
}

//Abb函数返回url关键字类型，如http://,ftp://....
int  Abb(char input[MaxLength], int length,char payload[MaxLength])
{
    int i=0,j=0,h=0,k=0;
    int result=0;
    char typelen[MaxLength][MaxLength];
    char type[35][30]=
    {
        "tel:","urn:","pop:","sip:",//长度为4
        "imap:","sips:","tftp:","news:",//5
        "ftp://","dav://","nfs://","smb://",//6
        "http://","ftps://","rtsp://","mailto://","sftp://","file://",//7
        "https://","btspp://","urn:epc:","urn:nfc:",//8
        "telnet://","btgoep://",//9
        "ftp://ftp.","btl2cap://","tcpobex://",//10
        "http://www.","irdaobex://","urn:epc:id:",//11
        "https://www.","urn:epc:tag:","urn:epc:raw:","urn:epc:pat:",//12
        "ftp://anonymous:anonymous@"//26
    };
    uint8_t value[35]=
    {
        0x05,0x13,0x14,0x15,
        0x11,0x16,0x17,0x0F,
        0x0D,0x0E,0x0C,0x0B,
        0x03,0x09,0x12,0x06,0x0A,0x1D,
        0x04,0x18,0x22,0x23,
        0x10,0x1A,
        0x08,0x19,0x1B,
        0x01,0x1C,0x1E,
        0x02,0x1F,0x21,0x20,
        0x07
    };
    //长度小于12时，求url关键字
    if (length<=12)
    {
        for(i=length; i>=4; i--)
        {
            for(j=0; j<i; j++)
                typelen[i][j]=input[j];
            for(k=34; k>=0; k--)
            {
                if(!strcmp(typelen[i],type[k]))
                {
                    for(j=i,h=0; j<=length; j++,h++)
                        payload[h]=input[j];
                    result=value[k];
                    return result;
                }
            }
        }
        return result;
    }
    //长度大于12小于26时求url关键字
    else if(length>12&&length<26)
    {
        for(i=12; i>=4; i--)
        {
            for(j=0; j<i; j++)
                typelen[i][j]=input[j];
            for(k=34; k>=0; k--)
            {
                if(!strcmp(typelen[i],type[k]))
                {
                    for(j=i,h=0; j<=length; j++,h++)
                        payload[h]=input[j];
                    result=value[k];
                    return result;
                }
            }
        }
        return result;
    }
    //长度大于等于26时求url关键字
    else
    {
        for(j=0; j<26; j++)
            typelen[26][j]=input[j];
        for(k=34; k>=0; k--)
        {
            if(!strcmp(typelen[26],type[k]))
            {
                for(j=26,h=0; j<=length; j++,h++)
                    payload[h]=input[j];
                result=value[k];
                return result;
            }
        }
        for(i=12; i>=4; i--)
        {
            for(j=0; j<i; j++)
                typelen[i][j]=input[j];
            for(k=34; k>=0; k--)
            {
                if(!strcmp(typelen[i],type[k]))
                {
                    for(j=i,h=0; j<=length; j++,h++)
                        payload[h]=input[j];
                    result=value[k];
                    return result;
                }
            }
        }
        return result;
    }
}


int ndef_URL(char input[MaxLength],uint8_t Record[MaxLength],int offset)
{
    int j=0,len,length,k;
    uint8_t RecordTypeLength=0x01;
    uint8_t RecordName=0x55;
    int Abbre;
    int PayloadLength;
//payload表示字符串去掉http://的那部分
    char payload[MaxLength];
//求输入字符串的长度
    for(j=0; input[j]!='\0'; j++);
    length=j;
    //求url关键字类型，如http://,ftp://....
    Abbre=Abb(input,length,payload);
    //求有效载荷的长度
    for(j=0; payload[j]!='\0'; j++);
    PayloadLength=j+1;
    //求参数
    Record[1]=RecordTypeLength;
    if(PayloadLength<256)
    {
        if(offset==0)
        {
            Record[0]=0xD1;
        }
        else if(offset==1)
        {
            Record[0]=0x91;
        }
        else if(offset==2)
        {
            Record[0]=0x11;
        }
        else if(offset==3)
        {
            Record[0]=0x51;
        }
        else
        {
            return -1;
        }
        Record[2]=PayloadLength;
        Record[3]=RecordName;
        Record[4]=Abbre;
        for(j=5,k=0; payload[k]!='\0'; j++,k++)
            Record[j]=(uint8_t)payload[k];
        len=PayloadLength+4;
    }
    else
    {
        if(offset==0)
        {
            Record[0]=0xC1;
        }
        else if(offset==1)
        {
            Record[0]=0x81;
        }
        else if(offset==2)
        {
            Record[0]=0x01;
        }
        else if(offset == 3)
        {
            Record[0]=0x41;
        }
        else
        {
            return -1;
        }
        Record[2]=(PayloadLength/256/256/256)%256;
        Record[3]=(PayloadLength/256/256)%256;
        Record[4]=(PayloadLength/256)%256;
        Record[5]=PayloadLength%256;
        Record[6]=RecordName;
        Record[7]=Abbre;
        for(j=8,k=0; payload[k]!='\0'; j++,k++)
            Record[j]=(uint8_t)payload[k];
        len=PayloadLength+7;
    }
    return  len;

}


void ndef_transfer_interface(char INPUT[][MaxLength],uint8_t final_result[])
{

    int offset; //offset为0表示0xD1/C1，1表示91/81，2表示11/01，3表示51/41
    int ip = 0;
    int i,k=0;
    int num=0;
    int length = 0;
    int final_length =0;
    char input[MaxLength][MaxLength];
    char type[MaxLength];
    uint8_t result[MaxLength][MaxLength];
   for(i=0;INPUT[i][0]=='t'||INPUT[i][0]=='u';i++)
    {
     type[i]=INPUT[i][0];
       for(k=2;INPUT[i][k];k++)
        {
            input[i][k-2]=INPUT[i][k];
              }
    }
         num=i;
   if(num==1)
    {
        offset=0;
        if(type[0]=='t')
        {

            length=ndef_Text(input[0],result[0],offset);
        }
        else if(type[0]=='u')
        {

            length=ndef_URL(input[0],result[0],offset);
        }
               final_length = length;
      }

    else if(num == 2)
    {
        for(ip = 0; ip<num; ip++)
        {
            if(ip == 0)
            {
                offset = 1;
                if(type[ip] == 't')
                    length=ndef_Text(input[ip],result[ip],offset);
                else if(type[ip] == 'u')
                    length=ndef_URL(input[ip],result[ip],offset);
            }
            else
            {
                offset = 3;
                if(type[ip] == 't')
                    length=ndef_Text(input[ip],result[ip],offset);
                else if(type[ip] == 'u')
                    length=ndef_URL(input[ip],result[ip],offset);
            }
            final_length =final_length + length;
                   }
    }

    else if(num > 2)
    {
        for(ip = 0; ip<num; ip++)
        {
            if(ip == 0)
            {
                offset = 1;
                if(type[ip] == 't')
                    length=ndef_Text(input[ip],result[ip],offset);
                else if(type[ip] == 'u')
                    length=ndef_URL(input[ip],result[ip],offset);
            }
            else if (ip == num-1)
            {
                offset = 3;
                if(type[ip] == 't')
                    length=ndef_Text(input[ip],result[ip],offset);
                else if(type[ip] == 'u')
                    length=ndef_URL(input[ip],result[ip],offset);
            }
            else
            {
                offset = 2;
                if(type[ip] == 't')
                    length=ndef_Text(input[ip],result[ip],offset);
                else if(type[ip] == 'u')
                    length=ndef_URL(input[ip],result[ip],offset);
            }
            final_length =final_length + length;
                }
    }

    if(final_length < 256)
    {
        final_result[0] = 0x00;
        final_result[1] = final_length;
    }
    else
    {
        final_result[0] = (final_length/256)%256;
        final_result[1] = final_length%256;
    }

    int tmp = 2;
    int tmp_outer,tmp_inner;
    for(tmp_outer = 0;tmp_outer < num;tmp_outer++)
    {
        for(tmp_inner = 0;result[tmp_outer][tmp_inner]!='\0';tmp_inner++)
        {
            final_result[tmp] = result[tmp_outer][tmp_inner];
            tmp++;
        }
    }
}


