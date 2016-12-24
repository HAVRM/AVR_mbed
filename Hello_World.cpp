#include "mbed.h"
#include "gps-sd.h"
//#include "SDFileSystem.h"

/*DigitalOut LED(PB_0);
int main(void){
  LED=0;
  wait_ms(500);
  LED=1;
}*/

//Serial pc;
//DigitalOut led(PB_0);

//FATFS ff;   /* 論理ドライブのワーク エリア(ファイル システム オブジェクト) */
//FATFileSystem ss("sd");
/*
int main (void)
{
  for(int i=0;i<5;i++){
    led=1;
    wait_ms(500);
    led=0;
    wait_ms(500);
  }
  led=1;
  //char *s;
  //FILE *fp=fopen("/sd/test.txt","a");
  //fputc('a',fp);
  //fputs("asdffffff",fp);
  //fgets(s,25,fp);
//    FIL fil;       /* ファイル オブジェクト */
//    char line[82]; /* 行バッファ */
//    FRESULT fr;    /* 戻り値 */


    /* デフォルト ドライブにワークエリアを与える */
//    f_mount(&ff, "", 0);

    /* テキスト ファイルを開く */
//    fr = f_open(&fil, "message.txt", FA_READ);
//    if (fr) return (int)fr;

    /* 1行ずつ読み出して表示 */
//    while (f_gets(line, sizeof line, &fil))
//        pc.printf("%s",line);

    /* ファイルを閉じる */
//    f_close(&fil);
  //fclose(fp);
/*  led=0;
  return 0;
}
*/

BusOut col(PC_0,PC_1,PC_2,PC_3,PC_4);
//DigitalOut tes1(PC_4);
//Digital
BusOut row(PD_2,PD_3,PD_4,PD_5,PD_6,PD_7,PB_7);
DigitalIn sw(PB_1);
DigitalOut dip(PB_6);

int main(void){
  int i=0;
  int k=0;
  while(1){
    for(int o=0;o<7;o++){
      for(int n=0;n<45;n++){
        dip=(o+n/5)%2;
        for(int m=0;m<100;m++){
          k%=7;
          i=0;
          //for(int l=0;l<5;l++)i=i|(JMP(k+o*7,l+n)<<(4-l));
          for(int l=0;l<5;l++)i=i|(JMP(k,l)<<(4-l));
          col=0;
          row=~(1<<(6-k));
          col=i;
          //col=place_col_5bit(139.580,35.584,k);
          wait_ms(1);
          k++;
        }
      }
    }
  }
  return 0;
}
