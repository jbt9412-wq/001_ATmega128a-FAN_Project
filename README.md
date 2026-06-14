# ATmega128A 기반 스마트 선풍기 제어 시스템

ATmega128A 기반의 스마트 선풍기 제어 프로젝트입니다. 하드웨어 제어부와 로직을 기능별로 완전 모듈화하고, 리모컨 제어 및 실시간 모니터링 환경을 구축했습니다.

## 프로젝트 개요

- **기간:** 2026.05.08 ~ 2026.05.12
- **참여 인원:** 3명
- **담당 역할:** IR/UART 로직 분리 설계 및 인터럽트 자원 충돌 디버깅, 모듈 통합(Merge)

## 주요 기능

- DC 모터 PWM 제어를 통한 4단계(0~3단) 풍속 조절
- 서보 모터 PWM 제어를 통한 스윙(회전) 모드 지원
- NEC IR 프로토콜 디코딩을 통한 리모컨 원격 호출 및 제어
- UART 통신 기반 PC 터미널 실시간 상태 모니터링 및 제어
- 7-Segment FND를 이용한 현재 풍속 단계 표시
- 서보 모터 위치와 동기화된 LED Bar 잔상(Sweep) 효과 구현
- 소프트웨어 디바운싱이 적용된 물리 버튼 조작

## 시스템 개요

펌웨어는 기능별 분할 모듈(`.c`, `.h`)로 작성되었습니다. `main.c`에 15ms 지연 기반의 메인 폴링 루프 스케줄러를 구축하여 인터럽트 자원 경합을 방지합니다. 무거운 제어 연산이 ISR을 점유하지 않도록 입력 스캔과 출력 갱신 태스크를 엄격히 분리하여 시스템 반응성을 최적화했습니다.

Timer1은 하드웨어 Fast PWM으로 모터 구동을 담당하며, Timer0은 소프트웨어 PWM을 통해 디스플레이 시각 효과를 연산합니다. IR 프로토콜 디코딩은 INT4와 Timer3를 결합하여 비동기식으로 처리됩니다.

## 하드웨어

| 분류 | 모듈/장치 |
| --- | --- |
| MCU | ATmega128A |
| 통신 | UART-USB 변환 모듈 |
| 수신 | NEC IR 적외선 수신부 |
| 표시 장치 | 공통 캐소드 7-Segment FND, 8채널 LED Bar |
| 구동 장치 | DC 모터 (풍속), 서보 모터 (회전) |
| 입력 장치 | 택트 스위치 (풍속/회전 제어용) |

## 주요 핀/주변장치 매핑

| 기능 | 주변장치/핀 |
| --- | --- |
| 풍속/회전 버튼 | `PA0`, `PA1` (Input, 내부 풀업) |
| IR 수신부 | `PE4` (`INT4` 하강 에지 트리거) |
| 서보 모터 | `PB5` (`Timer1 OC1A` Fast PWM 50Hz) |
| DC 모터 | `PB6` (`Timer1 OC1B` Fast PWM 50Hz) |
| FND (풍속 표시) | `PORTC` (Output) |
| LED Bar (상태 표시)| `PORTD` (Output, `Timer0` S/W PWM) |
| UART 통신 | `USART0` |
| IR 펄스 측정 | `Timer3` |

## 프로젝트 구조

```text
001_ATmega128a-FAN_Project/
├── .vscode/               # IDE 환경 설정 (tasks.json)
├── Makefile               # 빌드 스크립트 자동화 파일
├── .gitignore             # 빌드 부산물 업로드 방지
├── README.md              # 프로젝트 명세서
├── main.c                 # 메인 폴링 루프 스케줄러
├── 01_fan_config.h        # 전역 상태 플래그 및 라이브러리
├── 02_fan_register.c/.h   # 기본 GPIO 방향 및 풀업 설정
├── 03_fan_uart.c/.h       # IR 프로토콜 디코딩 및 UART0 통신
├── 04_fan_LED_FND.c/.h    # FND 및 S/W PWM 기반 LED 제어
├── 05_fan_motor.c/.h      # 하드웨어 PWM 기반 서보/DC 제어
└── 06_fan_button.c/.h     # 버튼 디바운싱 및 상태 천이
```

## 주요 소스 파일

| 파일 | 역할 |
| --- | --- |
| `main.c` | 하드웨어 초기화 시퀀스 및 메인 폴링 루프 (입력/출력 태스크 분리) |
| `01_fan_config.h` | 시스템 전역 상태 변수(`speed_state`, `rotate_state` 등) 외부 참조 |
| `02_fan_register.c` | I/O 레지스터(DDR, PORT) 초기화 |
| `03_fan_uart.c` | Timer3 펄스 폭 측정을 통한 IR 디코딩 로직, UART 송수신 |
| `04_fan_LED_FND.c` | FND 세그먼트 패턴 출력, Timer0 OVF 인터럽트 기반 LED 밝기 제어 |
| `05_fan_motor.c` | 풍속별 DC 모터 듀티비 매핑, 서보 모터 구동 범위 연산 |
| `06_fan_button.c` | 30ms 딜레이 기반 S/W 엣지 검출 및 채터링 제거 알고리즘 |

## 빌드 환경

필요 도구:

- AVR GCC Toolchain (`avr-gcc`)
- Make
- AVRDUDE (플래시 업로드용)
- Visual Studio Code

터미널에서 `Makefile`을 통해 빌드합니다.

```bash
# 펌웨어(.hex) 빌드
make all

# 생성된 부산물 삭제
make clean
```

ISP 프로그래머를 연결한 상태에서 타겟 보드에 업로드할 때 아래 명령어를 사용합니다. (Makefile 내 `PROGRAMMER` 변수 설정 필요)

```bash
make flash
```

## 사용 방법

1. 소스 코드를 로컬 환경에 Clone 하고 VS Code 워크스페이스로 엽니다.
2. 하드웨어 핀 매핑 테이블을 참고하여 ATmega128A와 각 모듈을 결선합니다.
3. 터미널에서 `make all` 명령을 실행하거나 VS Code 단축키(`Ctrl+Shift+B`)로 컴파일을 수행합니다.
4. ISP 장비 연결 후 `make flash`로 `.hex` 펌웨어를 보드에 업로드합니다.
5. PC에 Serial Monitor(9600bps)를 연결하여 부팅 메시지 및 상태 변경 로그를 확인합니다.

## 동작 로직 요약

1. 부팅 후 15ms 주기의 메인 루프가 시작되며, UART, IR, 버튼 입력 플래그를 폴링 방식으로 스캔합니다.
2. PA0 버튼, UART 입력, 또는 IR 리모컨(`SPEED_UP`/`DOWN`) 신호가 감지되면 전역 변수 `speed_state`를 0~3단으로 갱신합니다.
3. PA1 버튼, UART 입력, 또는 IR 리모컨(`ROTATE`) 신호가 감지되면 전역 변수 `rotate_state`를 토글합니다.
4. 출력 태스크에서 `speed_state`에 따라 Timer1 OC1B의 듀티비를 변경하여 DC 모터 속도를 제어하고 FND 숫자를 갱신합니다.
5. `rotate_state`가 활성화되면 서보 모터 제어값(OC1A)을 증감시켜 스윙 동작을 수행하고, 현재 위치에 비례하여 8개의 LED Array 밝기 배열을 재연산합니다.
6. Timer0 오버플로우 인터럽트가 갱신된 밝기 배열을 바탕으로 소프트웨어 PWM을 구동하여 LED 잔상 효과를 출력합니다.
