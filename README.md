# ATmega128A 기반 선풍기 제어 시스템

> **ATmega128A MCU를 활용하여 하드웨어 제어부와 로직을 기능별로 모듈화하고, 리모컨 제어 및 실시간 모니터링 환경을 구축한 시스템입니다.**

---

프로젝트 개요
- **진행 기간:** 2026.05.08 ~ 2026.05.12
- **참여 인원:** 3명 (개인 담당 역할: IR/UART 로직 설계 및 하드웨어 트러블슈팅)
- **개발 환경:** Ubuntu Linux, VS Code, avr-gcc
- **타겟 장치:** ATmega128A (16MHz)

---

## 🛠️ 사용 기술 및 장치 (Tech Stack)
- **Language:** C (AVR-GCC)
- **Peripherals:** Timer/Counter(0 & 1 & 3), External Interrupt (INT4), UART0
- **Hardware:** DC Motor (Fan), Servo Motor (Swing), NEC IR Receiver, FND Display, LED Bar(8ch)

---

## 🧩 소프트웨어 아키텍처 (Software Architecture)
기능별로 로직 레이어를 철저히 분리하여 코드의 유지보수성과 가독성을 극대화했습니다.

```text
001_ATmega128a-FAN_Project/
├── main.c                 # 메인 폴링 루프 스케줄러 (전체 태스크 관리)
├── 01_fan_config.h        # 전역 상태 플래그 및 공통 라이브러리 정의
├── 02_fan_register.c/.h   # 기본 GPIO 입출력 방향 및 내부 풀업 설정
├── 03_fan_uart.c/.h       # NEC IR 프로토콜 디코딩 및 UART0 통신 제어
├── 04_fan_LED_FND.c/.h    # FND 출력 및 8채널 소프트웨어 PWM 제어
├── 05_fan_motor.c/.h      # 타이머1 하드웨어 PWM 기반 서보/DC 모터 제어
└── 06_fan_button.c/.h     # GPIO 폴링 기반 버튼 입력 및 디바운싱
