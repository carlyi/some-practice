/**********************************************************************
Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
文件名称: cli_main.c
文件描述: OTT产测接口

修订记录:
        1. 作者: yiyuanping
           日期: 2014-06-17
           内容: 创建文件

**********************************************************************/
//#include <sys_status.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>

#define SERVER_PORT 6000   
#define SERVER_PORT_APK 6001   
#define BUFF_LEN 4096
#define TRUE "true"
#define FALSE "false"

int s = -1;    //创建全局套接字
int s_apk = -1;    //创建全局套接字

/*************************************************************************
Function:      * splitStr(char *pOrg, char **ppDest)
Description:   把字符串中间的空格去掉分段存放到数组里面
Input:         char *pOrg                    需要处理的字符串
                    char **ppDest                分段后存放的位置
Output:        无
Return:        pOrg
Others:
*************************************************************************/
char * splitStr(char *pOrg, char **ppDest){
     int i = 1, j = 0;
     int length = strlen(pOrg);
    
     *ppDest = pOrg;

     while(i<length){
          if(pOrg[i]=='\r' || pOrg[i]=='\n')
          {
               pOrg[i] = 0;
               break;
          }
          if((pOrg[i-1] == ' '||pOrg[i-1]==0)  && pOrg[i]!=' '){
               j++;
               ppDest[j] = &pOrg[i];
          }
          if(pOrg[i]==' ' || pOrg[i]=='\t'){
               pOrg[i] = 0;
          }
          i++;
     }
    
     return (length==0)?NULL:pOrg;
}

/*
函数功能:打印字符串数组到屏幕上!
*/
void printStr(char **ppDest){
     int i = 0;
     for(i = 0; i < 8; i++){
          if(ppDest[i]!=NULL)printf("%s, %d\n", ppDest[i], i);
     }
}

/*HELP info*/
void printHelp(){
     printf("\n please input the right operate command like this:\n");
     printf(" protest --mac -w 00112233445566 \n");
     printf(" protest --mac -r \n");
     printf(" protest --sn -w XZB123456 \n");
     printf(" protest --sn -r \n");
     printf(" protest --software -r \n");
     printf(" protest --reboot \n");
     printf(" protest --hello \n");
     printf(" protest --android +OTTCommand \n");
}


/************************************************************
Function:      int COMM_SystemEx(const char *pszCmd,char * pszResult,int iCount)
Description:   获取shell命令输出信息
Input:         const char *pszCmd,              要执行的命令
                   char * pszResult,                    执行的输出结果
                   int iCount                                当前buffer的长度
Output:
Return:        false ,       失败;
                    true,     成功;
Others:
************************************************************/
int SystemEx(const char *pszCmd,char * pszResult,int iCount)
{
    FILE   *stream;

    //memset( pszResult, '\0', sizeof(pszResult) );
    memset( pszResult, '\0', iCount );
    stream = popen( pszCmd, "r" );
    if (!stream)
    {
        pclose( stream );
        return 0;
    }

    iCount = fread( pszResult, sizeof(char), iCount, stream);
    if (!(iCount))
    {
        pclose( stream );
        return 0;
    }

    pclose( stream );
    return 1;
}


//HEX check: 0~9, a~f, A~F
int IsHex(char c){
     switch(c){
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
          case 'a':
          case 'b':
          case 'c':
          case 'd':
          case 'e':
          case 'f':
          case 'A':
          case 'B':
          case 'C':
          case 'D':
          case 'E':
          case 'F':
               return 0;
               break;
          default:
               return -1;
               break;
     }

     return -1;
}

//eg. 001122334455
int CheckMac( char *mac)
{
     int length = strlen(mac);
     int i = 0;
    
     if(mac == NULL)
          return -1;
    
     if (length != 12)
          return -1;
    
     for(i = 0 ; i < 12 ; i++)
     {
          if(IsHex(mac[i]))
          {
               return -1;
          };
     }
     return 0;
    
}

char *MacChange(char *SrcMac , char *DesMac)
{
     sprintf(DesMac ,"%c%c:%c%c:%c%c:%c%c:%c%c:%c%c" , SrcMac[0], SrcMac[1], SrcMac[2], SrcMac[3], SrcMac[4], SrcMac[5],
          SrcMac[6], SrcMac[7], SrcMac[8], SrcMac[9], SrcMac[10], SrcMac[11]);
     return DesMac;
}

int SendBack(char * BackResult , char * ClientAddress)
{
     int length = (BackResult==NULL)?0:strlen(BackResult);
     if(length == 0){
          printf("SendBack send length is 0.\n");
          strcpy(BackResult, "No send back data!!");
     }
     if(-1 == sendto(s ,BackResult , BUFF_LEN, 0, (struct sockaddr *)ClientAddress, sizeof(struct sockaddr_in)))
     {
          printf("sendto 'result' error!!");
          return -1;
     }
    
     return 0;
    
}

char *MacOperate(char **CommandValue ,char * ResultValue)
{
     if(!CommandValue[3-1])
     {
          printHelp();
          strcpy(ResultValue, "MAC operate command error!!");
          return NULL;
     }
     else{
          if(!strcmp(CommandValue[3-1],"-r"))
          {
               char Cmd[] = "efuse_tool mac read";
               SystemEx(Cmd,ResultValue,BUFF_LEN);
               printf("%s\n", ResultValue);    
          }
          else if (!strcmp(CommandValue[3-1],"-w"))
          {
               if (!CommandValue[4-1])
               {
                    strcpy(ResultValue, "Mac Error!! write mac fail");
                    return NULL;
               }
               else
               {
                    if (-1 == CheckMac(CommandValue[4-1]))
                    {
                         printf("\n Mac Error!! \n");
                         strcpy(ResultValue, "Mac Error!! write mac fail");
                         return NULL;
                    }
                    else
                    {
                         char Mac[32] = { 0 };
                         char Cmd[64] = { 0 };
                         memset(Mac, 0, sizeof(Mac));
                                            
                         MacChange(CommandValue[4-1], Mac);
                         sprintf(Cmd , "efuse_tool mac %s" , Mac);
                         SystemEx(Cmd, ResultValue, BUFF_LEN);
                         printf("%s\n", ResultValue);    
                    }
               }
              
          }
          else
          {
               printHelp();
              
               strcpy(ResultValue, "Mac operate command error!!");
               return NULL;
          }
     }
    
     return ResultValue;
    
}

char *WifiMacOperate(char **CommandValue ,char * ResultValue)
{
     if(!CommandValue[3-1])
     {
          printHelp();
          strcpy(ResultValue, "WifiMac operate command error!!");
          return NULL;
     }
     else{
          if(!strcmp(CommandValue[3-1],"-r"))
          {
               char Cmd[] = "cat sys/class/net/wlan0/address";
               SystemEx(Cmd,ResultValue,BUFF_LEN);
               printf("%s\n", ResultValue);    
          }
          else if (!strcmp(CommandValue[3-1],"-w"))
          {
               if (!CommandValue[4-1])
               {
                    strcpy(ResultValue, "Mac Error!! write wifimac fail");
                    return NULL;
               }
               else
               {
                    if (-1 == CheckMac(CommandValue[4-1]))
                    {
                         printf("\n WifiMac Error!! \n");
                         strcpy(ResultValue, "Mac Error!! write wifimac fail");
                         return NULL;
                    }
                    else
                    {
                         char Mac[32] = { 0 };
                         char Cmd[64] = { 0 };
                         memset(Mac, 0, sizeof(Mac));
                                                                
                         MacChange(CommandValue[4-1], Mac);
                         //strcpy(Mac , CommandValue[4-1]);
                         sprintf(Cmd , "efuse_tool wifi_mac %s" , Mac);
                         SystemEx(Cmd, ResultValue, BUFF_LEN);
                         //sprintf(ResultValue, "wlan0    efuse_set:mac,%s\n" , CommandValue[4-1]);
                         printf("%s\n", ResultValue);    
                    }
               }
          }
          else
          {
               printHelp();
              
               strcpy(ResultValue, "WifiMac operate command error!!");
               return NULL;
          }
     }
    
     return ResultValue;
    
}

char *SnOperate(char **CommandValue ,char * ResultValue)
{
     if(!CommandValue[3-1])
     {
          printHelp();
          strcpy(ResultValue, "SN operate command error!!");
          return NULL;
     }else {
          if(!strcmp(CommandValue[3-1],"-r"))
          {
               char Cmd[] = "efuse_tool userid read";
               SystemEx(Cmd, ResultValue, BUFF_LEN);
               printf("%s\n", ResultValue);    
          }
          else if (!strcmp(CommandValue[3-1],"-w"))
          {
               if (!CommandValue[4-1])
               {
                    strcpy(ResultValue, "Sn Error!!");
                    return NULL;
               }
               else
               {
                    char Cmd[64] = { 0 };
                        
                    sprintf(Cmd , "efuse_tool userid %s" , CommandValue[4-1]);
                    SystemEx(Cmd, ResultValue, BUFF_LEN);
                    printf("%s\n", ResultValue);    
               }
          }
          else
          {
               printHelp();
              
               strcpy(ResultValue, "Sn operate command error!!");
               return NULL;
          }
     }
    
     return ResultValue;
    
}

char *SoftVersionOperate(char **CommandValue ,char * ResultValue)
{
     if(!CommandValue[3-1])
     {
          printHelp();
          strcpy(ResultValue, "software version check operate command error!!");
          return NULL;
     }
     else
     {
          if(!strcmp(CommandValue[3-1], "-r"))
          {
               char Cmd[] = "getprop ro.product.firmware";
               SystemEx(Cmd, ResultValue, BUFF_LEN);
               printf("%s\n", ResultValue);    
          }
          else
          {
               printHelp();
               strcpy(ResultValue, "software version check operate command error!!");
               return NULL;
          }

     }
     return ResultValue;
}

char *ConnectionTest(char **CommandValue ,char * ResultValue)
{
     if (!CommandValue[3-1])
     {
          strcpy(ResultValue, "Connection is OK!!");
          return ResultValue;
     }
    
     else
     {
          printHelp();
          strcpy(ResultValue, "command error!!");
          return ResultValue;
     /*
          char Cmd[64] = { 0 };
                   
          sprintf(Cmd , "ping %s" , CommandValue[4-1]);
          SystemEx(Cmd, ResultValue, BUFF_LEN);
          printf("%s\n", ResultValue);
     */    
     }
}

char *RebootCommand(char **CommandValue ,char * ResultValue)
{
     char Cmd[] = "reboot";
     SystemEx(Cmd, ResultValue, BUFF_LEN);
     return ResultValue;
}

char *AndroidCommand(char **CommandValue ,char * ResultValue)
{
     if (CommandValue[3-1] &&  strncmp(CommandValue[3-1] , "checkgain" , strlen(CommandValue[3-1])) \
          && strncmp(CommandValue[3-1] , "checkthroughput" , strlen(CommandValue[3-1])))
     {
          char Cmd[128] = { 0 };
          if(CommandValue[4-1])
          {
               if(CommandValue[5-1])
               {
                    if (CommandValue[6-1])
                    {
                   
                         /*如果"--android"传进来多于四个参数，则提示命令输入错误*/
                         if(CommandValue[7-1])   
                         {
                              printHelp();
                              strcpy(ResultValue, "command error!!");
                              return NULL;
                         }
                        
                         //protest --android  ***  ***  ***   ***
                         else          
                         {
                              sprintf(Cmd , "%s %s %s %s" , CommandValue[3-1], CommandValue[4-1], CommandValue[5-1], CommandValue[6-1]);
                              SystemEx(Cmd, ResultValue, BUFF_LEN);
                              printf("%s\n", ResultValue);
                              return ResultValue;
                         }
                    }
                   
                    //protest --android  ***  ***  ***
                    else     
                    {
                         sprintf(Cmd , "%s %s %s" , CommandValue[3-1], CommandValue[4-1], CommandValue[5-1]);
                         SystemEx(Cmd, ResultValue, BUFF_LEN);
                         printf("%s\n", ResultValue);
                         return ResultValue;
                    }
               }
              
               //protest --android ***  ***
               else      
               {
                    sprintf(Cmd , "%s %s" , CommandValue[3-1], CommandValue[4-1]);
                    SystemEx(Cmd, ResultValue, BUFF_LEN);
                    printf("%s\n", ResultValue);
                    return ResultValue;
               }
          }
         
          //protest --android ***
          else  
          {
               sprintf(Cmd , "%s" , CommandValue[3-1]);
               //system(Cmd);
               SystemEx(Cmd, ResultValue, BUFF_LEN);
               printf("%s\n", ResultValue);
               return ResultValue;
               //return NULL;
          }
         
     }
     else if (CommandValue[3-1] && !strncmp(CommandValue[3-1] , "checkgain" , strlen(CommandValue[3-1])))
     {
          char Cmd[128] = "rtwpriv wlan0 efuse_get realmap |grep 0x10 |grep -v 0x100 |awk '{print $1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13}'";
          SystemEx(Cmd, ResultValue, BUFF_LEN);
          printf("%s\n", ResultValue);
     }
     else if (CommandValue[3-1]  && !strncmp(CommandValue[3-1] , "checkthroughput" , strlen(CommandValue[3-1])))
     {
          if(CommandValue[4-1])
          {
               char Cmd[128] = { 0 };
               sprintf(Cmd , "iperf -c %s -w 256k -r -P 2" , CommandValue[4-1]);
               SystemEx(Cmd, ResultValue, BUFF_LEN);
               printf("%s\n", ResultValue);
          }
          else
          {
               printHelp();
               strcpy(ResultValue, "android command error!!");
               return NULL;
          }
     }
     /*如果"--android"后面不接着输入OTT命令参数，则提示命令输入错误*/
     else    
     {
          printHelp();
          strcpy(ResultValue, "android command error!!");
          return NULL;
     }
    
     return ResultValue;
}

int main (int argc, char **argv)
{

     char *command[8] = {NULL};
     fd_set rfd;     // 读描述符集 
     int nRet; // select返回值 
     int maxfd;
    
     int n = -1;
     int n_apk = -1;
     char buff[BUFF_LEN];
     socklen_t len;
     len = sizeof(struct sockaddr_in);
    
     struct sockaddr_in client_address;
     bzero((void *)&client_address, sizeof(client_address) ); /*address to empty*/
     struct sockaddr_in server_address;
     bzero((void *)&server_address, sizeof(server_address) );   /*address to empty*/
     s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);   /*creating socket*/
     server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);         /*server PORT*/
     if (-1 == bind(s, (struct sockaddr *)&server_address, sizeof(struct sockaddr)))
          printf("bind  error!!");

     struct sockaddr_in client_address_apk;
     bzero((void *)&client_address_apk, sizeof(client_address_apk) ); /*address to empty*/
     struct sockaddr_in server_address_apk;
     bzero((void *)&server_address_apk, sizeof(server_address_apk) );   /*address to empty*/
     s_apk = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);   /*creating socket*/
     server_address_apk.sin_family = AF_INET;
    server_address_apk.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address_apk.sin_port = htons(SERVER_PORT_APK);         /*server PORT*/
     if (-1 == bind(s_apk, (struct sockaddr *)&server_address_apk, sizeof(struct sockaddr)))
          printf("bind  error!!");
    
     while(1)
     {
          FD_ZERO(&rfd); // 在使用之前总是要清空 
          maxfd = 0;

          // 开始使用select 
        FD_SET(s, &rfd); // 把socka放入要测试的描述符集中 
        FD_SET(s_apk, &rfd); // 把sockb放入要测试的描述符集中 
       
        maxfd = (s > s_apk )? s+1: s_apk+1;
         
        nRet = select(maxfd, &rfd, NULL, NULL, NULL);// 检测是否有套接口是否可读 
          if (nRet == -1)    
        { 
            printf("select() error/n"); 
            return -1; 
        }
          else if (nRet == 0) // 超时 
        { 
            printf("timeout/n"); 
            //closesocket(s); 
            //closesocket(s_apk); 
            break; 
        } 
          else
          {
               if (FD_ISSET(s, &rfd))
               {
                    memset(buff, 0, BUFF_LEN);
                    n = recvfrom(s, buff, BUFF_LEN, 0, (struct sockaddr *)&client_address, &len);
                    if (n == -1) 
                { 
                    printf("recvfrom() error!/n"); 
                    break; 
                }
               }

               if (FD_ISSET(s_apk, &rfd))
               {
                    memset(buff, 0, BUFF_LEN);
                    n_apk= recvfrom(s_apk, buff, BUFF_LEN, 0, (struct sockaddr *)&client_address_apk, &len);
                    if (n_apk== -1) 
                { 
                    printf("recvfrom() error!/n"); 
                    break; 
                }
               }
          }

          {         
               printf("server recved:%s\n", buff);

               memset(command, 0, sizeof(command));

               /*Parse the command from client, and make it section to the array command[]*/
               splitStr(&buff[0], &command[0]);

               /*print the command[] array to serial, for debug*/
               printStr(&command[0]);    
              
               char Result[BUFF_LEN];
               memset(Result, 0, sizeof(Result));
              
               /*Parse the command from client, do the operation for different modules */
               if(!strcmp(command[1-1],"protest"))
               {
                    /*MAC operate*/
                    if(!strcmp(command[2-1],"--mac"))
                    {
                         MacOperate(command, Result);    
                    }
                   
                    /*WifiMAC operate*/
                    else if(!strcmp(command[2-1],"--wifimac"))
                    {
                         WifiMacOperate(command, Result);    
                    }
                    /*SN  operate*/
                    else if(!strcmp(command[2-1],"--sn"))
                    {
                         SnOperate(command, Result);    
                    }
                    /*software version check*/
                    else if(!strcmp(command[2-1],"--software"))
                    {
                         SoftVersionOperate(command, Result);    
                    }
                    /*Connection check*/
                    else if(!strcmp(command[2-1],"--hello"))
                    {
                         ConnectionTest(command, Result);    
                    }
                    /*reboot*/
                    else if(!strcmp(command[2-1],"--reboot"))
                    {
                         RebootCommand(command, Result);    
                    }
                    /*especially for android command*/
                    else if(!strcmp(command[2-1],"--android"))
                    {
                         AndroidCommand(command, Result);    
                    }
                    else
                    {
                         printHelp();
                   
                         strcpy(Result, "operate command error!!");
                    }



               }
               else if(!strcmp(command[1-1],"RemoteTest"))
               {
                    if(!strcmp(command[2-1],"left"))
                    {
                         strcpy(Result, "left button test!");
                         printf("left button test!========== =========\n");
                    }
                    else if(!strcmp(command[2-1],"right"))
                    {
                         strcpy(Result, "right button test!");
                         printf("right button test!========== =========\n");
                    }
                    else if(!strcmp(command[2-1],"up"))
                    {
                         strcpy(Result, "up button test!");
                         printf("up button test!========== =========\n");
                    }
                    else if(!strcmp(command[2-1],"down"))
                    {
                         strcpy(Result, "down button test!");
                         printf("down button test!========== =========\n");
                    }
                    else if(!strcmp(command[2-1],"ok"))
                    {
                         strcpy(Result, "ok button test!");
                         printf("ok button test!========== =========\n");
                    }
                    else
                    {
                         printHelp();         
                         strcpy(Result, "operate command error!!");
                         printf("left button test!========== =========\n");
                    }
               }
               else
               {
                    printHelp();
                   
                    strcpy(Result, "operate command error!!");
               }

               /*send the reselt to client*/
               SendBack(Result , &client_address );


          }
     }

     return 0;
}

