//program drive BLDC fixed PWM
//Hall effect PIN_B3-->Blue:PIN_b4-->Green:PIN_b5-->Yellow

// save project on the wold



#include <30F2010.h>
#device adc=10
#Fuses  HS  
//#Fuses  BORV20
#Fuses  BROWNOUT
#Fuses  noMCLR
#Fuses  PUT4
#Fuses  NOWDT
#fuses WPSB10
#use delay(clock=20000000,restart_wdt)
#use rs232(UART1,baud=19200,parity=N,bits=8,)
//#include <math.h>
//#FUSES HPOL_low //High.Side Transistors Polarity is Active.High (PWM 1,3,5 and 7)
//#FUSES LPOL_low //Low.Side Transistors Polarity is Active.Low (PWM 0,2,4 and 6)
//----------------------------------------------------------------------

#priority TIMER1,EXT1

int16 trisB;
#locate trisB=0x02C6
#bit    trisb3=trisb.3
#bit    trisb4=trisb.4
#bit    trisb5=trisb.5
struct sensor{
    INT NON  :3; 
    int  data :3;
}hall_data;

#locate hall_data=0x02C8



int16 PTCON; 
#locate PTCON = 0x1C0 
#bit PTEN     = PTCON.15
#bit PTSIDL   = PTCON.13

#bit PTOPS3 = PTCON.7
#bit PTOPS2 = PTCON.6
#bit PTOPS1 = PTCON.5
#bit PTOPS0 = PTCON.4

#bit PTCKPS1 = PTCON.3 
#bit PTCKPS0 = PTCON.2
#BIT PTMOD1  =PTCON.1
#BIT PTMOD0  =PTCON.0


//-----------------------------------
INT16 PTMR;
#LOCATE  PTMR = 0X1C2

//-----------------------------------
INT16 PTPER;
#LOCATE PTPER = 0X1C4

//-----------------------------------
INT16 SEVTCMP;
#LOCATE SEVTCMP = 0X1C6 

//-----------------------------------
INT16 PWMCON1;
#locate PWMCON1 = 0x1c8
#bit PMOD3 = PWMCON1.10 
#bit PMOD2 = PWMCON1.9
#bit PMOD1 = PWMCON1.8
#bit PEN3H = PWMCON1.6
#bit PEN2H = PWMCON1.5
#bit PEN1H = PWMCON1.4

#bit PEN3L = PWMCON1.2
#bit PEN2L = PWMCON1.1
#bit PEN1L = PWMCON1.0
//-----------------------------------
INT16 PWMCON2;
#locate PWMCON2 = 0x1ca
#bit SEVOPS3 = PWMCON2.11 
#bit SEVOPS2 = PWMCON2.10 
#bit SEVOPS1 = PWMCON2.9
#bit SEVOPS0 = PWMCON2.8 
#bit OSYNC   = PWMCON2.1
#bit UDIS    = PWMCON2.0

//----------------------------------
INT16 DTCON1;
#LOCATE DTCON1 = 0X1CC
#BIT DTAPS1 = DTCON1.7
#BIT DTAPS0 = DTCON1.6

#BIT DTIME5 = DTCON1.5
#BIT DTIME4 = DTCON1.4
#BIT DTIME3 = DTCON1.3
#BIT DTIME2 = DTCON1.2
#BIT DTIME1 = DTCON1.1
#BIT DTIME0 = DTCON1.0
//----------------------------------
int16 ifs2;
#locate ifs2=0x088
#bit   fltaif = ifs2.11
INT16 FLTACON;
#LOCATE FLTACON = 0X1D0
#BIT    FAOV3H  = FLTACON.13
#BIT    FAOV3L  = FLTACON.12
#BIT    FAOV2H  = FLTACON.11
#BIT    FAOV2L  = FLTACON.10
#BIT    FAOV1H  = FLTACON.9
#BIT    FAOV1L  = FLTACON.8

#BIT    FLTAM   = FLTACON.7

#BIT    FAEN3   = FLTACON.2
#BIT    FAEN2   = FLTACON.1
#BIT    FAEN1   = FLTACON.0
//---------------------------------
INT16 OVDCON;
#LOCATE OVDCON  =  0X1D4
#BIT    POVD3H  =  OVDCON.13
#BIT    POVD3L  =  OVDCON.12
#BIT    POVD2H  =  OVDCON.11
#BIT    POVD2L  =  OVDCON.10
#BIT    POVD1H  =  OVDCON.9
#BIT    POVD1L  =  OVDCON.8

#BIT    POUT4H  =  OVDCON.7
#BIT    POUT3H  = OVDCON.5
#BIT    POUT3L  = OVDCON.4
#BIT    POUT2H  = OVDCON.3
#BIT    POUT2L  = OVDCON.2
#BIT    POUT1H  = OVDCON.1
#BIT    POUT1L  = OVDCON.0
//-----------------------------------
INT16 PDC1;
#LOCATE PDC1 = 0X1D6
INT16 PDC2;
#LOCATE PDC2 = 0X1D8
INT16 PDC3;
#LOCATE PDC3 = 0X1DA
INT16 CONST TABLE_FW[]={0x0000,0x2001,0x0810,0x0801,0x0204,0x2004,0x0210};
INT16 CONST TABLE_RW[]={0x0000,0x0210,0x2004,0x0204,0x0801,0x0810,0x2001};
INT8 INDEX,TEMPINDEX;
int1 flg_int,flg_t4,FLG_RDA;
int16 n;
INT32 GET_T23;
int16 duty;
INT8 GET_DUTY;
int16 loop;
#int_RDA
void  RDA_isr(void) 
{

GET_DUTY=GETC();
FLG_RDA=1;
}

#int_EXT1
void  EXT1_isr(void) 
{
flg_int=1;
GET_T23=get_timer23();
 set_timer23(0);
}

#int_TIMER1
void  TIMER1_isr(void) 
{set_timer1(500);
set_adc_channel( 0 );
duty = read_adc();
pdc1= pdc2= pdc3=duty;
flg_t4=1;
clear_interrupt(int_timer1);
}


void main(void)
{ setup_wdt (WDT_OFF);
  flg_t4=0;
  //trisb=0x000f;
  trisb3=trisb4=trisb5=1;
  ptper=0x0080;
  SEVTCMP=ptper;
  ptmr=0x0000;
  //===============PMOD3(PWM3 MODE) PMDO2(PWM2MODE) AND PMOD1(PWM1MODE) FOR SELECT COMPLEMENTARY OR Independent mode
  PMOD3=1;//PWM3
  PMOD2=1;//PWM2 
  PMOD1=1;// PWM1
  
  pten=1;      // ENABLE INPUT CLOCK FOR PWM  
  PTSIDL=0; 
  //===========PTMOD0 AND PTMOD1 FOR SELECT MODE PWM
  ptmod0=0;    // 00 Free Running mode
  ptmod1=0;    // 01  Single-shot mode
               // 10  Continuous Up/Down Counting mode.
               // 11   Continuous Up/Down mode with interrupts for double PWM
  //============PTCKPS0 AND PTCKPS1 BIT FOR PTMR PRESCALE
  ptckps0=1;   // 00 (1:1 prescale)
  ptckps1=0;   // 01 (1:4 prescale) 
               // 10 (1:16 prescale)
               // 11 (1:64 prescale)  
  //============PENxh PENxl for enable pwmoutput===============
  pen3h=1;    //   
  pen2h=1;    //
  pen1h=1;    //
  pen3l=1;    //
  pen2l=1;    //
  pen1l=1;    //
 
//=======================================
  pdc1=0x120; //
  pdc2=0x120; //
  pdc3=0x120; //

//=======================
   OSYNC=1;UDIS=0;
 
   POVD1H =1;
   POVD1L =1;
   POVD2H =1;
   POVD2L =1;

   POVD3H =1;
   povd3l =0;
   Pout3L =1;
//========================
 PWMCON2=0X0F00;

   //FAEN1=1;
   //FAOV1H=1;

   //FLTAM=0;
  INDEX=0;
  SETUP_ADC_PORTS(sAN0|VSS_VDD );
  SETUP_ADC(ADC_CLOCK_INTERNAL);
  enable_interrupts(INTR_GLOBAL);
  ENable_interrupts(INT_TIMER1);
  disable_interrupts(INT_RDA);
  set_timer1(500);
  //=====================SET TIMER23 32 BIT=======
  SETUP_TIMER2(TMR_EXTERNAL |TMR_DIV_BY_1|TMR_32_BIT);
  set_timer23(0);
  
  setup_timer1(TMR_INTERNAL|TMR_DIV_BY_8);
  disable_interrupts(INT_EXT1);
  ext_int_edge( 1, H_TO_L);
  pdc1= pdc2= pdc3= 20;
  flg_int=0;FLG_RDA=0;
  n=0;duty=200;
  OVDCON=TABLE_FW[0];
  WHILE(N<5)
  {
   OUTPUT_TOGGLE(PIN_d1);
   DELAY_MS(1000);
   N++;
  }
  OUTPUT_HIGH(PIN_d1);loop=0;
  
  while(true)  
{

  
INDEX=hall_data.data;
  while(INDEX==hall_data.data)
  {
   OVDCON=TABLE_FW[INDEX];

    n++;

    if(n>30000)
    {
    output_toggle(PIN_d1);
    n=0;
    }

  }

  

 

  
 }

   

  

}



