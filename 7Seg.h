#ifndef	_7SEGMENT_OUT_
#define _7SEGMENT_OUT_

#define pinA 2   // 회로구성의 편의성을 위해
#define pinB 3    // pin번호를 설정
#define pinC 4
#define pinD 5
#define pinE 6
#define pinF 8
#define pinG 9
#define pinDP 10



extern int pindef[8]={pinA,pinB,pinC,pinD,pinE,pinF,pinG, pinDP};
extern int seg_data[10][8]=
{
  {1,1,1,1,1,1,0,0},
  {0,1,1,0,0,0,0,0},
  {1,1,0,1,1,0,1,0},
  {1,1,1,1,0,0,1,0},
  {0,1,1,0,0,1,1,0},
  {1,0,1,1,0,1,1,0},
  {1,0,1,1,1,1,1,0},
  {1,1,1,0,0,1,0,0},
  {1,1,1,1,1,1,1,0},
  {1,1,1,1,0,1,1,0}
  };

void FND_Init(){
	for(int i=0 ; i<7; i++){
		pinMode(pindef[i], OUTPUT);
		digitalWrite(pindef[i],1);
	}
}

void OUT_SEG(int inputNum){
	for(int i=0; i<7; i++){
		digitalWrite(pindef[i],(seg_data[inputNum][i]==1) ? 0 : 1);
	}
}

#endif
