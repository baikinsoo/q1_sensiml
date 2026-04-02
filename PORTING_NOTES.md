# `touch_ml_porting_fin` 포팅/연결 정리

## 목표
- 터치(QTouch/PTC) 센서 입력을 SensiML Knowledge Pack(라이브러리)로 스트리밍해 **분류 결과가 UART 콘솔로 출력**되게 만들기.

---

## 프로젝트 구조(핵심)
- **MPLAB X 프로젝트**: `firmware/test_samd21.X`
  - 디바이스: `ATSAMD21J18A`
- **터치 측정(QTouch)**: `firmware/src/config/default/touch/`
  - 측정 주기: `DEF_TOUCH_MEASUREMENT_PERIOD_MS = 20ms` (≈ 50Hz)
  - 채널/센서 수: `DEF_NUM_CHANNELS=2`, `DEF_NUM_SENSORS=2`
  - 측정 완료 플래그: `measurement_done_touch`
  - 측정 루프 호출: `touch_process()`
- **SensiML(Knowledge Pack)**
  - 헤더: `firmware/src/sensiml/inc/*.h`
  - 라이브러리: `firmware/src/sensiml/lib/libsensiml.a`
  - 실행 래퍼: `firmware/src/sml_recognition_run.c`

---

## `gestures-demo` vs `touch_ml_porting_fin` 차이(“브리지 레이어” 관점)

### `gestures-demo`
- 센서 ISR → **ringbuffer에 샘플 적재**
- 메인 루프에서 ringbuffer에서 샘플을 읽어 **`sml_recognition_run(sample, axes)`로 스트리밍**
- 결과 출력 모듈(`sml_output`) 등 “센서↔ML 연결 다리”가 명확함

### `touch_ml_porting_fin` (초기 상태)
- `touch_process()`로 측정 수행
- `measurement_done_touch`가 1이 되면 **값을 `printf`로 찍기만** 하고, 모델로 전달하는 “다리”가 없었음
- 단, SensiML 라이브러리/헤더/링크 자체는 이미 구성되어 있었음
  - `-I"../src/sensiml/inc"` include 경로 포함
  - 링크 단계에서 `..\src\sensiml\lib\libsensiml.a` 포함

---

## SensiML 결과(숫자 vs 라벨 문자열)
- `kb_run_model()` / `sml_recognition_run()`의 리턴은 기본적으로 **클래스 ID(숫자)**.
- 하지만 모델에는 **클래스 맵(라벨 문자열)** 정보가 포함됨.
  - `firmware/src/sensiml/inc/model_json.h`에 `ClassMaps` 존재.
  - 예시(현재 모델): `0=Unknown`, `1=finger`, `2=noise`, `3=water`

### `kb_output.h` vs `kb.h`
- **`kb.h`**: 모델 실행 핵심 API (`kb_model_init`, `kb_run_model`, `kb_reset_model` 등)
- **`kb_output.h`**: 결과를 문자열/디버그 형태로 출력하는 helper (`kb_sprint_model_result` 등)
- `kb_sprint_model_result()`는 소스(.c)로는 안 보일 수 있고, 보통 **`libsensiml.a` 내부에 구현**되어 있음.

---

## 샘플 주기(20ms)와 분류 출력 주기
- 20ms는 **“샘플이 생성되는 주기”**(센서 측정/업데이트)이고,
- 분류 결과는 모델의 **윈도우/세그먼터/슬라이드 설정**에 의해 결정됨.
  - `kb_run_model()`은 “한 시점 샘플(1 timepoint)”을 계속 받아 내부 버퍼에 쌓고,
  - 윈도우가 만족되는 시점부터 일정 간격으로 분류가 발생함.

---

## `ptr++` 패턴(gestures-demo 코드에서 보인 이유)
- ringbuffer에서 “연속된 여러 샘플”을 받을 때,
  - `ptr`이 **샘플 배열의 시작**을 가리키고,
  - `ptr++`로 **다음 샘플 프레임**으로 이동하면서 1개씩 `sml_recognition_run()`에 넣음.
- 터치 프로젝트처럼 “측정 완료 이벤트마다 샘플 1개”를 바로 만들면, `ptr++` 같은 순회는 필요 없음.

---

## 적용한 변경 사항(최소 브리지 구현)
> 목적: 터치 입력을 SensiML 모델로 스트리밍하고 콘솔에 결과를 보이게 함.

### 변경 파일
- `firmware/src/main.c`

### 변경 내용 요약
- 모델 초기화 추가: `kb_model_init()` 1회 호출
- `measurement_done_touch` 발생 시:
  - **ch0 delta**를 1채널 샘플(`int16_t sample[1]`)로 구성
    - delta = `signal - reference`
  - `sml_recognition_run(sample, 1)` 호출
  - `ret >= 0`이면 콘솔에 `ml_class_id,<id>` 한 줄 출력

### 참고(이미 존재하던 출력)
- `firmware/src/sml_recognition_run.c`는 분류가 발생하면
  - `kb_sprint_model_result(...)`로 문자열을 구성하고 `printf("%s\n", ...)`로 출력하도록 구현되어 있음.
  - 그래서 콘솔에는 “포맷된 결과 문자열”과 “ml_class_id”가 함께 보일 수 있음.

---

## 다음 단계(필요 시)
- **채널 수 확정**: 학습이 1채널이면 `nsensors=1` 유지, 2채널이면 `sample[2]`로 확장
- **입력 스케일/특징 정합**: 학습에 사용한 입력이 `delta`인지, `signal`인지, 정규화가 있는지 확인
- **버퍼링 필요 시**: 샘플 유실/타이밍 문제가 있으면 `gestures-demo`처럼 ringbuffer를 도입해 안정화

