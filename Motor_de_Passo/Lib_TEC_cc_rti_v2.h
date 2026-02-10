/*
Data: setembro/2025
Versão Lib_TEC_cc_rti_v1.h
Com interrupção, mas sem aninhamento

Agora a rotina de varredura do teclado está associada à interrupção gerada pelos
pinos P2.2, P2.1 e P2.0 (colunas da matriz) quando houver uma borda de subida em
qualquer uma destas.

Nesta implementação, o atendimento da interrupção e a execução da varredura da
matriz do teclado acontecem muitas vezes mais rápido do que o tempo de
da tecla. Isto quer dizer que se nenhuma providência for tomada para levar em
consideração este fato, teremos inúmeras interrupções para um mesmo pressionar 
de tecla.

O resultado da varredura do teclado será salva na variável global inicializada
"char Tecla = 0"

*/

char Tecla = 0;

/*
Data: junho/2025
Versão Lib_TEC_cc_v1.h 
Inclusão da compilação condicional

A intenção é criar um conjunto de funções que sejam compiladas de acordo
com o processador selecionado. Para tal vamos utilizar alguns comandos
direcionados ao compilador que permitem implementar o que é conhecido como
compilação condicional.

*/


/*
Data: julho/2024
Versão antiga

Nesta versão a leitura do teclado é feita por polling (ou espera ocupa, em portugês)
Algumas adições e mdoficações foram feitas nas funções originais do professor
Joel.

1. Inclusão da funçao Espera_tecla()
2. Compilação condicional para ajustar a função correta para os kits 1611 e 149
*/


/* Título: bllbioteca de funcões do teclado kit MSP430F1611
Autor: Joel A. Santos
Data: abril/2019
Ambiente: IAR Embeded Workbench IDE; ver.: 5.5.2.1
última rev: 04/2019
------------------------------------------------------------------------------*/
/* Título: Leitura de teclado matricial por polling
Autor: Joel A. Santos
Data: agosto/2020
Ambiente: IAR Embeded Workbench IDE; ver.: 5.60.2
última rev: 06/2021
------------------------------------------------------------------------------*/


/* 
Hardware do kitF1611
Linhas  L0 = P3.0 ;  L1 = P3.1;  L2 = P3.2;  L3 = P3.3;
Colunas C0 = P2.0;   C1 = P2.1;  C2 = P2.2  

   Versão para o kitF149
Linhas  L0 = P6.4 ;  L1 = P6.5;  L2 = P6.6;  L3 = P6.7;
Colunas C0 = P2.0;   C1 = P2.1;  C2 = P2.2  
*/

//==============================================================================
// Pacote de código a ser compilado se o processador selecionado for 
#if defined (__MSP430F1611__) //Versão para o kitF1611

// Configura os pinos de I/O para atender o teclado
void tec_config_rti()
{
  P2SEL &= ~(BIT0+BIT1+BIT2);     
  P2DIR &= ~(BIT0+BIT1+BIT2);       //Define P2.0,P2.1 e P2.2 como entrada - colunas
  P2IE  |=  (BIT0+BIT1+BIT2);       // Habilita a INT gerada por estes bits
  P2IES &= ~(BIT0+BIT1+BIT2);       // Estes bits geram INT na borda de subida
  
  P3SEL &= ~(BIT0+BIT1+BIT2+BIT3);  // Configura P2 e P3 como I/O  
  P3DIR |=  (BIT0+BIT1+BIT2+BIT3);  // define linhas em P3 como saída
  P3OUT |=  (BIT0+BIT1+BIT2+BIT3);  // Seta estes bits para permiter a geração
                                    // de borda de subida em P2 quando uma tecla
                                    // qualquer for pressionada.
  
}

// Converte o código de varredura obtido na função tec_rd em ASCII
char ascii_convert(char key_code)
{
  char tab_tec[12] = {"0123456789AE"};
  char tab_key[12] = {0x28,0x41,0x21,0x11,0x42,0x22,0x12,0x44,0x24,0x14,0x48,0x18};
  unsigned int i=0;
  unsigned flag =0; // sinalisador de conversão de tecla
  
  while ( flag==0x00)
  {
    if ( key_code != tab_key[i])
    {
      i++;
    }
    else
      flag=0xFF;
  }
  return tab_tec[i];
}

//Faz a varredura do teclado e gera como saída o código ASCII da tecla presssionada.
//SE não houver tecla pressionada retorna 0x00
char tec_rd()
{
  char col = 0, lin = BIT0, key_code=0, tecla=0;
  
   // Permanece em loop até encontrar uma coluna ativa
  // ou até o termino da varredura
  while ((col==0x00) & (lin < BIT4))
  {
    P3OUT = P3OUT &~(BIT0+BIT1+BIT2+BIT3); // Zera todas as saídas
    P3OUT = P3OUT | lin; //ativa uma linha
    
    col = P2IN &(BIT0+BIT1+BIT2); //atualiza estado das colunas
    
    if (col!=0x00)  // Tem alguma tecla pressionada?
    {
      // Sim, tem tecla pressionada
      col= col << 4;// concatenar linha com coluna
      key_code=lin+col;
      tecla = ascii_convert(key_code); //conveerter em ASCII
    }
    else
      // Não tem tecla pressionada
      lin = lin << 1; //ativar próxima linha
  }
  return (tecla); // retorna ASCII da tecla
}

// Rotina de Tratamento da Interrupção da porta P2
//Versão para o kitF1611
#pragma vector=PORT2_VECTOR
__interrupt void TEC_rti()
{
  // Antes de executar esta função o processador executa automaticamente
  // um disable interrupt.
  
  // Desabilita a INT gerada em P2. Assim o periférico não reconhecerá 
  // as transiçoes de nível geradas pela trepidação da chave.
  P2IE  &=  ~(BIT0+BIT1+BIT2);       
  
  // Os dois procedimentos abaixo constituem a preparação para o aninhamento
  // de interrupções
  // Resseta os flags de INT
  // Isso garante que a própria INT da P2 não entre em loop de interrupções
  P2IFG &= ~(BIT0+BIT1+BIT2);
  
  // Agora esta RTI poderá ser interrompida pela RTI de qualquer outro periférico
  __enable_interrupt(); 
  
  // Aguarda a passagem do tempo correspondente à trepidação da tecla
  // Normalmente, um tempo de delay de 5ms é suficiente.
  __delay_cycles((long)(Fclk*5e-3));
  
  // Passada a trepidação, só faz a varredura se alguma coluna estiver ativa.
  // Isto quer dizer que a INT ocorreu no fechamento da tecla
  if (P2IN &(BIT0+BIT1+BIT2))
  {
    // Faz a varredura do teclado
    Tecla = tec_rd();
    
    // Seta estes bits para permiter a geração de borda de subida em P2 
    // quando uma tecla qualquer for pressionada.
    P3OUT |= (BIT0+BIT1+BIT2+BIT3);  
  }
  
  // Resseta os flags de INT
  P2IFG &= ~(BIT0+BIT1+BIT2);
  
  // Reabilita as novas INT geradas em P2. 
  P2IE  |=  (BIT0+BIT1+BIT2);       
    
  //Após o termino do processamento desta RTI o processador faz um enable
  // interrupt automático
} 

//Fim da versão para o kitF1611

//==============================================================================

#elif defined (__MSP430F149__) // Versão para o kitF149

/* Versão para o kitF149
Linhas  L0 = P6.4 ;  L1 = P6.5;  L2 = P6.6;  L3 = P6.7;
Colunas C0 = P2.0;   C1 = P2.1;  C2 = P2.2  
*/

// Configura os pinos de I/O para atender o teclado
void tec_config_rti()
{
  P2SEL &= ~(BIT0+BIT1+BIT2);       // Configura P2 como I/O
  P2DIR &= ~(BIT0+BIT1+BIT2);       //Define P2.0,P2.1 e P2.2 como entrada - colunas
  P2IE  |=  (BIT0+BIT1+BIT2);       // Habilita a INT gerada por estes bits
  P2IES &= ~(BIT0+BIT1+BIT2);       // Estes bits geram INT na borda de subida
  
  P6SEL &= ~(BIT4+BIT5+BIT6+BIT7);  // Configura P6 como I/O  
  P6DIR |=  (BIT4+BIT5+BIT6+BIT7);  // define linhas em P6 como saída
  P6OUT |=  (BIT4+BIT5+BIT6+BIT7);  // Seta estes bits para permiter a geração
                                    // de borda de subida em P2 quando uma tecla
                                    // qualquer for pressionada.
  
}

// Converte o código de varredura obtido na função tec_rd em ASCII
char ascii_convert(char key_code)
{
  // Nesta versão adaptada para o kit 149 o keycode teve inversão na posicão do 
  // códgio da linha com o da coluna
  char tab_key[12] = {0x82,0x14,0x12,0x11,0x24,0x22,0x21,0x44,0x42,0x41,0x84,0x81};
  char tab_tec[12] = {"0123456789AE"}; 
  unsigned int i=0;
  unsigned flag =0; // sinalisador de conversão de tecla
  
  while ( flag==0x00)
  {
    if ( key_code != tab_key[i])
    {
      i++;
    }
    else
      flag=0xFF;
  }
  return tab_tec[i];
}

//Faz a varredura do teclado e gera como saída o código ASCII da tecla presssionada.
//SE não houver tecla pressionada retorna 0x00
char tec_rd ()
{
  char col = 0, lin = BIT4, key_code=0, tecla=0, i = 0;
  
  // Permanece em loop até encontrar uma coluna ativa
  // ou até o termino da varredura
  while ((col==0x00) & (i < 4))
  {
    P6OUT = P6OUT &~(BIT4+BIT5+BIT6+BIT7); // Zera todas as saídas
    P6OUT = P6OUT | lin; //ativa uma linha
  
    col = P2IN &(BIT0+BIT1+BIT2); //atualiza estado das colunas
    
    if (col!=0x00)  // Tem alguma tecla pressionada?
    {
      // Sim, tem tecla pressionada      
      key_code=lin+col;                // concatenar linha com coluna
      tecla = ascii_convert(key_code); //conveerter em ASCII
    }
    else
    {
      // Não tem tecla pressionada
      lin = lin << 1; //ativar próxima linha
      i++;
    }
  }
  return (tecla); // retorna ASCII da tecla
}

// Rotina de Tratamento da Interrupção da porta P2
//Versão para o kitF149
#pragma vector=PORT2_VECTOR
__interrupt void TEC_rti()
{
  // Antes de executar esta função o processador executa automaticamente
  // um disable interrupt.
  
  // Desabilita a INT gerada em P2. Assim o periférico não reconhecerá 
  // as transiçoes de nível geradas pela trepidação da chave.
  P2IE  &=  ~(BIT0+BIT1+BIT2);       
  
  // Os dois procedimentos abaixo constituem a preparação para o aninhamento
  // de interrupções
  // Resseta os flags de INT
  // Isso garante que a própria INT da P2 não entre em loop de interrupções
  P2IFG &= ~(BIT0+BIT1+BIT2);
  
  // Agora esta RTI poderá ser interrompida pela RTI de qualquer outro periférico
  __enable_interrupt(); 
  
  // Aguarda a passagem do tempo correspondente à trepidação da tecla
  // Normalmente, um tempo de delay de 5ms é suficiente.
  __delay_cycles((long)(Fclk*5e-3));
  
  // Passada a trepidação, só faz a varredura se alguma coluna estiver ativa.
  // Isto quer dizer que a INT ocorreu no fechamento da tecla
  if (P2IN &(BIT0+BIT1+BIT2))
  {
    // Faz a varredura do teclado
    Tecla = tec_rd();
    
    // Seta estes bits para permiter a geração de borda de subida em P2 
    // quando uma tecla qualquer for pressionada.
    P6OUT |= (BIT4+BIT5+BIT6+BIT7);  
  }
  
  // Resseta os flags de INT
  P2IFG &= ~(BIT0+BIT1+BIT2);
  
  // Reabilita as novas INT geradas em P2. 
  P2IE  |=  (BIT0+BIT1+BIT2);       
    
  //Após o termino do processamento desta RTI o processador faz um enable
  // interrupt automático
} 

//Fim da versão para o kitF149

//==============================================================================

#else
#error "****** Voce esqueceu de selecionar o processador adequado. *****"
#error "****** Selecione MSP430F1611 ou MSP430F149.                *****"

#endif
//==============================================================================


// Função que lê uma uma tecla e só retorna quando o usuário
// liberar a tecla

// Qual o inconveniente que esta função gera?

char Espera_tecla()
{
  char tecla;
  
  tecla = tec_rd();
  
  if(tecla) while(tec_rd());
  
  return tecla;
  
}