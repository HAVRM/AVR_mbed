#include "mbed.h"
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

//BusOut tes1(PC_0,PC_1,PC_2,PC_3,PC_4);
DigitalOut tes1(PC_4);
DigitalOut tes2(PD_2);//,PD_3,PD_4,PD_6,PD_7,PB_7);
DigitalIn sw(PB_1);
DigitalOut din(PB_6);

int main(void){
  din=0;
  tes2=0;
  while(1){
    tes1=sw;
  }
}
