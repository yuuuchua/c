/*
* CEFET-MG — Campus II
*
* Aluna: Maria Alice Ribeiro Martins
* Data: 10/10/2025*
* Curso: Eletrônica
* Turma: ELT3CT2
*
* Projeto: Motor de Passo v3 (com encoder)
* Plataforma: MSP430
* IDE: IAR Embedded Workbench
*
* Descrição: Elaborar um programa que faça o controle do acionamento do kit motor de passo do nosso laboratório;
* Uma mensagem de abertura com a sua identificação, sua turma e o ano corrente deverá ser exibida, 
* durante 3 segundos, na primeira e na segunda linha do display.
*
* Serão utilizadas as teclas 2, 4, 5, 6 e 8. O display deverá estar sempre atualizado com a situação 
* do motor. Portanto, devem ser sempre exibidas a direção de giro e a velocidade.
*
* A indicação de velocidade deverá ser exibida na primeira linha do display no formato 
* “Velocidade = n” e a situação do motor (Parado, Esquerda, Direita) na segunda linha.
*
* A velocidade de giro do motor, deve ser de, no máximo 500 pps (intervalo de 2 ms), o que 
* corresponde ao valor 9. Os demais valores (8, 7, 6, ...., 1) devem apresentar uma relação linear.
*
* Logo após o RESET o motor deve estar parado e o valor da velocidade deve ser igual a 5.
*
* As teclas e suas funções são:
* Tecla Função “Mensagem no Display”
* 5 parar o motor “Parado”
* 4 girar para a esquerda “Esquerda”
* 6 girar para a direita “Direita”
* 2 aumentar a velocidade atualiza a velocidade
* 8 diminuir a velocidade atualiza a velocidade
*/

// Inclusão das bibliotecas
#include "io430.h"
#include "stdlib.h"
#include "Lib_MLCD.h"            // Biblioteca de controle do display MLCD 
#include "Lib_TEC_cc_rti_v2.h"   // Biblioteca de leitura do teclado matricial

// DEfinição de variáveis
#define PARADO 0
#define Esquerda -1
#define Direita 1

// Criação de variáveis globais
int VELOCIDADE = 0;
int GIRO = 0;

// Protótipos de funções
void AtualizaDisp(void);
void Config_TimerA(void);
void IniciaD(void);

void main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  
  // Chamada de funções de configuração de periféricos
  Config_TimerA();                   // Configura o WDT
  ConfigIO_MLCD();                // Configura MLCD
  Init_MLCD();                    // Inicia MLCD
  tec_config_rti();               // Configura teclado
  
  // Habilita interrupção
  __enable_interrupt(); 
  
  // Configuração de IOs
  P1SEL &= ~(BIT0+BIT1+BIT2+BIT3+BIT6+BIT7); // Seleciona como I/O
  P1DIR |= (BIT0+BIT1+BIT2+BIT3);            // Configura P1.0 a P1.3 como saída
  P1DIR &= ~(BIT6+BIT7);                     // Configura P1.6 e P1.6 como entradas
  P1OUT &= ~(BIT0+BIT1+BIT2+BIT3+BIT6+BIT7); // Limpa saídas e entradas
  
  // Estado inicial do display
  IniciaD();
  
  while(1)
  {
    // Se tecla pressionada
    if(Tecla)
    {
      switch (Tecla)
      {
        // Caso a tecla 5 for pressionada, o motor estará parado
      case '5':
        GIRO = PARADO;
        AtualizaDisp();
        //Para parar o timer
        TACTL &= ~MC0;
        break;
        // Caso a tecla 2 for pressionada, a velocidade do motor aumenta
      case '2':
        if (VELOCIDADE < 9)
        {
          VELOCIDADE++;
          //Para alterar a velocidade
          TACCR0 = ((10 - VELOCIDADE)*66);
          AtualizaDisp();
        }
        break;
        // Caso a tecla 8 for pressionada, a velocidade do motor diminui
      case '8':
        if (VELOCIDADE > 1)
        {
          VELOCIDADE--;
          //Para alterar a velocidade
          TACCR0 = ((10 - VELOCIDADE)*66);
          AtualizaDisp();
        }
        break;
        // Caso a tecla 4 for pressionada, o motor vai para a esquerda
      case '4':       
        GIRO = (-1);
        AtualizaDisp();
        //Para colocar o timer em funcionamento
        TACTL |= MC0 + TACLR;  
        break;
        // Caso a tecla 4 for pressionada, o motor vai para a direita
      case '6':
        GIRO = 1;
        AtualizaDisp();
        //Para colocar o timer em funcionamento
        TACTL |= MC0 + TACLR;       
        break;
      }
      Tecla = 0x00;
    }    
  } 
}

// Função de configuração do Timer A
void Config_TimerA(){
  //Configuração inicial do Timer_A: (parado)
  TACTL = TASSEL0 + TACLR + TAIE;
  TACCR0 = 66*5; // O valor *5 corresponde à condição inicial de velocidade = 5
}

// Função para escrever no display o nome do aluno e estado inicial do motor
void IniciaD()
{
  MLCD_LinhaColuna(1, 2);
  EnviaString("Maria Alice R. Martins");
  MLCD_LinhaColuna(2, 1);
  EnviaString("EletronicaC - T2");
  __delay_cycles(_250ms*8);
  MLCD_LinhaColuna(1, 2);
  EnviaString("Velocidade = 5");
  MLCD_LinhaColuna(2, 1);
  EnviaString("Estado = PARADO  ");
  VELOCIDADE = 5;
}

// Função de atualização do display
void AtualizaDisp()
{
  switch (Tecla)
  {
    // Caso a tecla 5 for pressionada, no display será indicado PARADO
  case '5':
    MLCD_LinhaColuna(2, 10);
    EnviaString("PARADO  ");
    break;
    // Caso a tecla 2 for pressionada, no display será indicada a velocidade aumentando
  case '2':
    MLCD_LinhaColuna(1, 15);
    EnviaDado(VELOCIDADE + '0');
    break;
    // Caso a tecla 8 for pressionada, no display será indicada a velocidade aumentando
  case '8':
    MLCD_LinhaColuna(1, 15);
    EnviaDado(VELOCIDADE + '0');
    break;
    // Caso a tecla 4 for pressionada, no display será indicado ESQUERDA
  case '4':
    MLCD_LinhaColuna(2, 10);
    EnviaString("ESQUERDA  ");
    break;
    // Caso a tecla 6 for pressionada, no display será indicado DIREITA
  case '6':
    MLCD_LinhaColuna(2, 10);
    EnviaString("DIREITA  ");
    break;
  }
}

// Interrupção pelo TimerA
#pragma vector = TIMERA1_VECTOR
__interrupt void acionamento()
{
  static int Step = 0; // Passo atual do motor
  static int Estado[2][4] = {{0x00, 0x00, 0x00, 0x00},  //Parado
  {0x0C, 0x06, 0x03, 0x09}}; //seq. crescente = Esquerda
  //seq. decrescente = Direita 
  
  P1OUT = Estado[abs(GIRO)][Step];    // Atualiza as bobinas do motor
  Step = ((GIRO+Step) & (BIT1+BIT0)); // Avança ou volta um passo
  
  TACTL &= ~TAIFG;                    // Reseta flag de interrupção
}

// Interrupção do encoder
#pragma vector = PORT1_VECTOR    
__interrupt void PORTA1_ISR (void){
  
  if(P1IN & BIT6){
    if(VELOCIDADE < 9)
    {
      VELOCIDADE++;
    }
  } else{
    if(VELOCIDADE > 1)
    {
      VELOCIDADE --; 
    }
  }
  P1IFG &=~ (BIT7);
}