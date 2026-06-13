# ATmega128A 기반 선풍기 제어 시스템

> **ATmega128A MCU를 활용하여 하드웨어 제어부와 로직을 기능별로 모듈화하고, 리모컨 제어 및 실시간 모니터링 환경을 구축한 시스템.**

---

프로젝트 개요
- **진행 기간:** 2026.05.08 ~ 2026.05.12
- **참여 인원:** 3명 (개인 담당 역할: IR/UART 로직 설계 및 하드웨어 트러블슈팅)
- **개발 환경:** Ubuntu Linux, VS Code, avr-gcc
- **타겟 장치:** ATmega128A (16MHz)

---

##  사용 기술 및 장치 (Tech Stack)
- **Language:** C (AVR-GCC)
- **Peripherals:** Timer/Counter(0 & 1 & 3), External Interrupt (INT4), UART0
- **Hardware:** DC Motor (Fan), Servo Motor (Swing), NEC IR Receiver, FND Display, LED Bar(8ch)

---

##  소프트웨어 아키텍처 (Software Architecture)
기능별로 로직 레이어를 분리하여 코드의 유지보수성과 추후 확장성을 대비


001_ATmega128a-FAN_Project/
├── main.c                 # 메인 폴링 루프 스케줄러 (전체 태스크 관리)
├── 01_fan_config.h        # 전역 상태 플래그 및 공통 라이브러리 정의
├── 02_fan_register.c/.h   # 기본 GPIO 입출력 방향 및 내부 풀업 설정
├── 03_fan_uart.c/.h       # NEC IR 프로토콜 디코딩 및 UART0 통신 제어
├── 04_fan_LED_FND.c/.h    # FND 출력 및 8채널 소프트웨어 PWM 제어
├── 05_fan_motor.c/.h      # 타이머1 하드웨어 PWM 기반 서보/DC 모터 제어
└── 06_fan_button.c/.h     # GPIO 폴링 기반 버튼 입력 및 디바운싱


# 🌀 ATmega128A 기반 스마트 선풍기 제어 시스템

> 하드웨어 제어부와 로직을 기능별로 완전 모듈화하고, 리모컨 제어 및 실시간 모니터링 환경을 구축한 임베디드 시스템입니다.

![System Architecture](여기에_시스템_블록도_이미지_링크_삽입)

<br>

## 🛠 Tech Stack
![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![Ubuntu Linux](https://img.shields.io/badge/Ubuntu-E95420?style=for-the-badge&logo=ubuntu&logoColor=white)
![VS Code](https://img.shields.io/badge/VS%20Code-0078d7.svg?style=for-the-badge&logo=visual-studio-code&logoColor=white)
![MCU](https://img.shields.io/badge/Target-ATmega128A-00A98F?style=for-the-badge)

- **Peripherals:** `Timer/Counter(0, 1, 3)`, `INT4 (External Interrupt)`, `UART0`
- **Hardware:** `DC Motor`, `Servo Motor`, `NEC IR Receiver`, `FND`, `LED Bar(8ch)`

<br>

## 📌 Project Overview
- **기간:** 2026.05.08 ~ 2026.05.12
- **참여 인원:** 3명
- **담당 역할:** IR/UART 로직 분리 설계 및 인터럽트 자원 충돌 디버깅

<br>

## 🔌 Hardware Pin Mapping
| Peripheral | ATmega128A Pin | Description |
| :--- | :--- | :--- |
| **버튼 (풍속/회전)** | `PA0`, `PA1` (Input) | 내부 풀업 및 30ms S/W 디바운싱 적용 |
| **NEC IR 수신부** | `PE4` (INT4) | 하강 에지 트리거 외부 인터럽트 입력 |
| **서보 모터 (회전)**| `PB5` (OC1A) | Timer1 하드웨어 Fast PWM (50Hz) |
| **DC 모터 (풍속)** | `PB6` (OC1B) | Timer1 하드웨어 Fast PWM (50Hz) |
| **FND (풍속 표시)** | `PORTC` (Output)| 공통 캐소드 세그먼트 패턴 출력 |
| **LED Bar (상태)** | `PORTD` (Output)| Timer0 S/W PWM 기반 8채널 개별 제어 |

<br>

## 💡 Key Implementation

### 1. 메인 폴링 루프 스케줄러 (Task Scheduler)
- 인터럽트 자원 경합을 방지하기 위해 `main.c`에 15ms 지연 기반의 메인 스케줄러 구축.
- 무거운 제어 연산이 ISR을 점유하지 않도록 입력 스캔과 출력 갱신 태스크를 엄격히 분리.

### 2. NEC IR 프로토콜 정밀 디코딩 (INT4 & Timer3)
- INT4 하강 에지 발생 시 Timer3 값을 읽어 펄스 폭 측정 (Leader Code 2000~5000 판별).
- 32비트 프레임 수신 완료 시 `ir_flag`만 1로 세팅하며, 모터 제어 로직은 폴링 루프에서 일괄 처리.

### 3. 하드웨어/소프트웨어 PWM 혼합 제어
- **Timer1 (H/W PWM):** 50Hz 제어 주기로 DC 모터(풍속 4단계) 및 서보 모터(250~500 범위) 독립 제어.
- **Timer0 (S/W PWM):** 오버플로우 인터럽트를 활용한 100주기 해상도로 서보 모터 위치에 동기화된 LED 잔상(Sweep) 효과 구현.

<br>

## 📁 Architecture
<details>
<summary><strong>👉 여기를 클릭하여 파일 모듈 구조 보기</strong></summary>
<br>

기능별로 로직 레이어를 분리하여 유지보수성을 극대화했습니다.
```text
001_ATmega128a-FAN_Project/
├── main.c                 # 메인 폴링 루프 스케줄러
├── 01_fan_config.h        # 전역 상태 플래그 및 라이브러리
├── 02_fan_register.c/.h   # 기본 GPIO 방향 및 풀업 설정
├── 03_fan_uart.c/.h       # IR 프로토콜 디코딩 및 UART0 통신
├── 04_fan_LED_FND.c/.h    # FND 및 S/W PWM 기반 LED 제어
├── 05_fan_motor.c/.h      # 하드웨어 PWM 기반 서보/DC 제어
└── 06_fan_button.c/.h     # 버튼 디바운싱 및 상태 천이
