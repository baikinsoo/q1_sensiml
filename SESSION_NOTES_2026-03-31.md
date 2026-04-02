# 세션 노트 (2026-03-31) — `touch_ml_porting_fin`

## 목표(현재까지 합의)
- QTouch/PTC 터치 센서 입력을 SensiML Knowledge Pack(`kb_*`)에 스트리밍해서 **UART 콘솔에 분류 결과가 보기 좋게 출력**되게 한다.

---

## Knowledge Pack 동작 핵심 정리
- `kb_run_model(pSample, nsensors, model_index)`는 **timepoint(샘플 1프레임)** 단위 입력을 받는 스트리밍 API.
- 호출될 때마다 내부적으로:
  - 샘플을 **모델 내부 ring buffer**에 누적
  - 세그멘테이션/특징/분류 조건이 만족되면 결과 반환
- 리턴값 의미(헤더 주석 기준):
  - `>= 0`: 분류 결과(클래스 ID)
  - `-1`: 아직 세그먼트/윈도우가 안 잡힘(대기)
  - `-2`: 세그먼트가 필터로 제거됨
- **외부 ringbuffer는 필수 아님**
  - `kb_run_model()` 자체가 **내부 ring buffer를 사용**함.
  - 외부 ringbuffer는 ISR/지터/유실 방지 등 “안정화”가 필요할 때 선택.

---

## QTouch 입력: `signal` vs `reference` vs `delta`
코드 근거: `firmware/src/config/default/touch/touch.c`
- `get_sensor_node_signal(n)`:
  - `ptc_qtlib_node_stat1[n].node_acq_signals`
  - **현재 측정된 원시값(raw acquisition)**
- `get_sensor_node_reference(n)`:
  - `qtlib_key_data_set1[n].channel_reference`
  - **무터치 상태의 기준선(baseline)** (드리프트/재보정으로 천천히 따라감)
- 보통 “터치 변화량”은:
  - `delta = signal - reference`
  - 무터치면 delta가 0 근처, 터치면 delta가 유의미하게 변함
- `reference`는 0이 정상값이 아님.
  - `reference`는 원시값 단위의 baseline이라 `signal`과 비슷한 크기(예: 500대)가 정상.

---

## 분류가 “안 바뀌는 것처럼 보였던” 주요 원인들
- `main.c`에서 **같은 measurement 이벤트에 모델을 3번 호출(delta/signal/reference)** 하던 구조가 있었음  
  → 스트리밍 상태가 꼬이고 입력 의미가 섞여 결과가 고정/이상해질 수 있음.
- 입력 표현 불일치 가능성:
  - 학습 데이터가 raw `signal` 기반인데, 펌웨어는 `delta`(대부분 0)를 넣으면 분류가 한 클래스로 치우칠 수 있음.
  - 실제 콘솔에서 `signal==reference`가 자주 보여 `delta≈0` 스트림이 모델 입력이 되는 경우가 확인됨.

---

## 콘솔 출력 개선(적용된 변경)
### 1) `main.c` (로그 정리 + 스로틀)
파일: `firmware/src/main.c`
- 모델 초기화 후 `expected_nsensors = kb_get_num_sensor_buffers(model_index)`를 읽고 부팅 로그로 출력.
- 입력 표현 스위치:
  - `ML_INPUT_MODE 0`: delta
  - `ML_INPUT_MODE 1`: raw signal
  - `ML_INPUT_MODE 2`: raw reference
- 출력 과다 문제를 줄이기 위해 **N번 측정마다 1번만 로그 출력**:
  - `LOG_EVERY_N_MEASUREMENTS` (기본 25 → 50Hz 기준 약 2Hz)

### 2) `sml_recognition_run.c` (라벨 출력)
파일: `firmware/src/sml_recognition_run.c`
- Knowledge Pack의 클래스 맵(현재 모델):
  - `1=finger`, `2=noise`, `3=water`
- 분류 발생 시(`ret>=0`) 숫자 대신 라벨 문자열 출력:
  - `ml_label,finger|noise|water|unknown`
- 라벨이 눈에 띄도록 앞뒤로 구분선 출력:
  - `--------------------`

---

## “default가 water로 찍히는” 현상 해석
- 모델이 “디폴트로 water를 내는” 게 아니라,
  - 펌웨어 입력이 `delta≈0` 근처로 고정되는 상황에서
  - 그 패턴이 모델에서 water(클래스 3) 쪽으로 분류되기 쉬워 **water가 자주 나오는** 것으로 해석.
- SensiML UI 캡처의 **Class Map(1=finger,2=noise,3=water)** 은 라벨 매핑의 정답표라 관련 있음.
- AIF 테이블은(패턴/학습 관련 파라미터 성격) 지금 증상의 1차 원인일 가능성은 낮음.

---

## 다음 할 일(내일 오전 목표: “터치 시작 후 N개만 feed”)
요구사항:
- “손가락이 닿자마자 20개 샘플만 수집”했더니 최대가 550 정도였고,
- 실제로 누르면 800까지 가는데, **‘변화 시작 이후’의 N개만 모델에 넣어 확인**하고 싶음.

핵심 결론:
- `kb_run_model()`은 “샘플 20개를 한 번에 넣는” API가 아니라 **샘플 1개씩 스트리밍**하는 API.
- 따라서 “20개를 통째로 넣는다”는 의미는 구현상:
  - (1) 내부 버퍼를 비우고(`kb_flush_model_buffer`)
  - (2) 변화 시작 감지 이후 N개 샘플을 **N번 연속 호출**로 feed
  - (3) 그 구간에서 나온 분류 중 “마지막 유효 분류(ret>=0)”만 한 번 출력

추가 체크포인트:
- 모델 윈도우가 20샘플보다 길면 20개로는 분류가 안 나올 수 있음  
  → N을 20→50/100 등으로 늘려서 테스트.
- 학습 입력이 `signal`인지 `delta`인지가 매우 중요  
  → `ML_INPUT_MODE`로 빠르게 전환 테스트.

