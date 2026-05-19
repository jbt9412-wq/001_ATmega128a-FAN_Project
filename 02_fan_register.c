#include "01_fan_config.h"
#include "02_fan_register.h"

void register_init(void) {
    /* [Input 설정] 조작 버튼 및 센서 */
    
    // PA0: 풍속 제어, PA1: 회전 제어 (내부 풀업 활성화)
    DDRA &= ~((1 << PA0) | (1 << PA1)); 
    PORTA |= (1 << PA0) | (1 << PA1);   

    // PE4: IR 리모컨 수신부 (INT4 외부 인터럽트, 내부 풀업 활성화)
    DDRE &= ~(1 << PE4); 
    PORTE |= (1 << PE4); 

    /* [Output 설정] 모터 및 디스플레이 */
    
    // PB5: 서보 모터(회전) PWM, PB6: DC 모터(풍속) PWM 출력
    DDRB |= (1 << PB5) | (1 << PB6); 

    // PORTC: 풍속 상태 표시용 FND 출력
    DDRC = 0xFF; 

    // PORTD: 회전 상태 표시용 LED 바 출력
    DDRD = 0xFF; 
}