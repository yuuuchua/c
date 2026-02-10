/*
* CEFET-MG — Campus II
*
* Aluna: Maria Alice Ribeiro Martins
* Data: 28/11/2025
* Curso: Eletrônica
* Turma: ELT3CT2
*
* Projeto: Prog13_Ex7: Dimmer Digital
* Plataforma: MSP430
* IDE: IAR Embedded Workbench
*
* Proposta: Elaborar um programa para o kit MSP430F1611 que funcione como um dimmer
* digital. Utilize as orientações de configuração dos registrados contidas na apostila de
* Laboratório do Prof. Marcos(Prática 19).
* - Deve-se sempre exibir o ângulo de disparo e o estado de ativação
* Na linha 1: Ang = xxx° 
* Na linha 2: Ligado ou Desligado
* - Utilizaremos as teclas:
* "Entra" para ativar o dimmer
* "Anula" para desativar o dimmer
* "2" para aumentar o angulo de disparo em incrementos unitários
* "5" para decrementar o angulo de disparo em decrementos unitários
* "3" para incrementos de 10 unidades de angulo
* "6" para decrementos de 10 unidades de angulo
* - Tenha atenção aos valores de ângulos de disparo mínimo e máximo. Ângulos muito
* baixos ou próximos as 180º podem gerar comportamento indesejado.
* - Determine os limites na prática. Inicie com a faixa de 5º a 175°.
*/

#include "io430.h"
#include "stdlib.h"
#include "Lib_MLCD.h"            // Biblioteca de controle do display MLCD
#include "Lib_TEC_cc_rti_v2.h"   // Biblioteca de leitura do teclado matricial

#define _1grau (int)((4000000 / 120) / 180) // Ticks do Timer A para um grau de atraso (~185)
#define _Pulsogate 40                       // Largura do pulso de disparo do TRIAC em ticks 

// Criação de variáveis globais
int ang = 5;    // Ângulo de disparo inicial em graus (limite mínimo)

// Protótipo de funções
void Config_IO();    
void Config_TimerA();
void Config_Clock();
void att_Display();

void main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  
  // Chamada de funções de configuração de periféricos
  Config_Clock();          // Configura o clock 
  Config_IO();             // Configura P1.1 e P1.2
  Config_TimerA();         // Configura o Timer A
  ConfigIO_MLCD();         // Configura IOs para o MLCD
  Init_MLCD();             // Inicia MLCD
  tec_config_rti();        // Configura teclado por interrupção
  
  // Habilita interrupção global
  __enable_interrupt();
  
  // Inicialização do display
  MLCD_LinhaColuna(1, 1);
  EnviaString("Ang: 005    ");
  MLCD_LinhaColuna(2, 1);
  EnviaString("Desligado    "); // Estado inicial
  
  char estado = 0; // Variável para evitar que o display seja atualizado toda hora
  
  while(1)
  {
    // Verifica se uma tecla foi pressionada
    if (Tecla)
    {
      switch (Tecla)
      {
      case 'E': // Tecla 'Entra' (Ligar)
        if (estado != 1)
        {
          MLCD_LinhaColuna(2, 1);
          EnviaString("Ligado       ");
        }        
        // Para colocar o timer em funcionamento
        TACTL = TASSEL_2 | MC_2;
        estado = 1;
        break;
      case 'A': // Tecla 'Anula' (Desligar)
        if (estado != 0)
        {
          MLCD_LinhaColuna(2, 1);
          EnviaString("Desligado    ");
        }
        estado = 0;
        // Para parar o timer
         TACTL = TASSEL_2 | MC_0 | TACLR;
        break;
      case '2': // Incremento unitário
        if (ang < 175)
        {
          ang++;
          att_Display();
        }
        break;
      case '5': // Decremento unitário 
        if (ang > 5)
        {
          ang--;
          att_Display();
        }
        break;
      case '3': // Incremento de 10
        if (ang < 165)
        {
          ang = (ang + 10);
          att_Display();
        }
        break;
      case '6': // Decremento de 10 
        if (ang > 10)
        {
          ang = (ang - 10);
          att_Display();
        }
        break;
      }
      Tecla = 0; // Limpa a variável global da tecla após processamento
    }
  }
}

// Função de configuração de IOs
void Config_IO()
{
  // Configura P1.1 e P1.2 como funções especiais 
  P1SEL |= (BIT1+BIT2);
  // Configura P1.2 como saída 
  P1DIR |= (BIT2);
  // Configura P1.1 como entrada
  P1DIR &= ~(BIT1);
  // Limpa as saídas/entradas
  P1OUT &= ~(BIT1+BIT2);
}

// Função de configuração do Timer A
void Config_TimerA()
{
  // TACTL:
  // TASSEL_2: SMCLK (4MHz)
  // MC_0: Modo inicialmente desligado
  // TACLR: Limpa o contador TAR
  TACTL = TASSEL_2 | MC_0 | TACLR;
  
  // Configurado para Capture
  TACCTL0 = CM_1 | CCIS_0 | CAP | SCS | CCIE;
  
  // Configurado para Compare
  TACCTL1 = OUTMOD_3;
}

// Função de configuração do Clock
void Config_Clock()
{
  // Liga o oscilador de alta frequência XT2
  BCSCTL1 &= ~XT2OFF;
  // Seleciona o SMCLK 
  BCSCTL2 |= SELS;
}

// Função base de atualização do display
void att_Display()
{
  int centena, dezena, unidade;
  
  // Divisão do ângulo em dígitos
  centena = ang / 100;           // Extrai a centena
  dezena = (ang % 100) / 10;     // Extrai a dezena
  unidade = ang % 10;            // Extrai a unidade
  
  // Atualiza o display com os três dígitos
  MLCD_LinhaColuna(1, 6);        // Vai para a coluna 6 da linha 1 
  EnviaDado(centena + '0');      // Exibe a centena (convertendo para ASCII)
  EnviaDado(dezena + '0');       // Exibe a dezena
  EnviaDado(unidade + '0');      // Exibe a unidade
}

// Interrupção pelo TimerA 
#pragma vector = TIMERA0_VECTOR
__interrupt void controle()
{
  // Calcula o tempo em que o pulso deve iniciar
  TACCR1 = TACCR0 + (ang * _1grau);
  
  // Define o tempo em que o pulso deve terminar
  TACCR0 = TACCR1 + _Pulsogate;
  
  // Limpa a flag de interrupção
  TACCTL0 &= ~CCIFG;
}