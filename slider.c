#include "ble.h"
#include "main.h"
#include "slider.h"

uint16_t sliderAdc[SIZE_ADC_SLIDER];

unsigned char valueSlider=0;
unsigned char sliderVariation = 0;

void initAdcSlider(void)
{ 
    /*----habilita o clock do DMA---*/
  RCC->AHBENR |= RCC_AHBENR_DMAEN;
  /*-- configuração do canal 1 - ADC--*/
  /*Endereço do periférico a ser lido (ADC)*/
  DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;
  /* Endereço da memória que conterá o dado lido */
  DMA1_Channel1->CMAR = (uint32_t)&sliderAdc[0];
  /*Quantidade de dados a transferir*/
  DMA1_Channel1->CNDTR = 2;
  /*Configuração do periférico*/
  DMA1_Channel1->CCR |= DMA_CCR_CIRC|DMA_CCR_MINC|DMA_CCR_PSIZE_0|DMA_CCR_MSIZE_0;
  /*Habilita o canal*/
  DMA1_Channel1->CCR |= DMA_CCR_EN ;
  
   /*GPIOA habilita clk*/
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN ;
  
    /*PA6 e PA5 - Entradas analógicas*/
  GPIOA->MODER |= GPIO_MODER_MODER5 | GPIO_MODER_MODER6;
  GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR1;
  
  /*Habilita o clock do ADC*/
  RCC->APB2ENR |= RCC_APB2ENR_ADCEN;
  
  /*----configuração do adc-------*/
  ADC1->CFGR1 |= ADC_CFGR1_WAIT|ADC_CFGR1_CONT; //continuous mode e wait conversion
  ADC1->CFGR2 |= ADC_CFGR2_CKMODE_1; //PCLK/2
  ADC1->CHSELR |= ADC_CHSELR_CHSEL5 |ADC_CHSELR_CHSEL6 ; //canais 6 e 5
  ADC->CCR |= ADC_CCR_VREFEN; //referência interna
  //ADC1->IER |= ADC_IER_EOCIE; //interrupção de conversão
  
  /*----calibração-----*/
  /*garante que ADEN=0*/
  ADC1->CR &= ~ADC_CR_ADEN;
  /*inicia calibração*/
  ADC1->CR |= ADC_CR_ADCAL;
  /*aguarda até a calibração estar concluída*/
  while(!(ADC1->CR - ADC_CR_ADCAL));
 
  /*-----habilitação do DMA-------*/
  ADC1->CFGR1 |= ADC_CFGR1_DMAEN;
  /*DMA circular*/
  ADC1->CFGR1 |= ADC_CFGR1_DMACFG;
    
  /*------procedimento para ligar o adc------*/
  /*liga o módulo*/
  ADC1->CR |= ADC_CR_ADEN;
  /*espera até o bit ADRDY indicar que o ADC está pronto*/
  while(!(ADC1->ISR - ADC_ISR_ADRDY)); 
}

void StartReadSlider(void)
{
  ADC1->CR |= ADC_CR_ADSTART;
}

void StopReadSlider(void)
{
  ADC1->CR |= ADC_CR_ADSTP;
}

