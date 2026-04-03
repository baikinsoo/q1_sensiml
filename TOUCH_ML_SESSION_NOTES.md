# Touch ML 포팅 — 세션 정리 노트

날짜: 2026-04-03 기준 대화 요약 (touch_ml_porting_fin / samd21_touch_ML / SensiML KP)

---

## 1. 프로젝트 구분

| 프로젝트 | 역할 |
|----------|------|
| **touch_ml_porting_fin** | 메인 포팅 타깃. `libsensiml.a`, 터치 캡처/스트림 `main.c`, MPLAB 링크 설정이 갖춰진 쪽. |
| **samd21_touch_ML** | 별도 폴더 프로젝트. `touch_ml_porting_fin`의 `main` 패턴을 이식·`libsensiml.a` 링크 이슈(IDE가 Makefile 재생성 시 `.a` 누락) 등을 다룸. |

---

## 2. `idle` 클래스 (4클래스 모델)

- 학습 메타데이터에 **finger / idle / noise / water** 같이 4클래스가 있으면 `model_json.h`의 `ClassMaps`와 `sml_class_label_from_raw_id()`를 동일하게 맞춰야 함.
- **터치 시작 후 N샘플만 넣는 캡처 모드**에서는 윈도우가 이미 “변화 구간”에 가깝기 때문에 **`idle`이 결과로 자주 의미 있게 나오지 않을 수 있음**. Continuous streaming에서는 배경 구분용으로 쓰기 쉬움.
- 배포 모델을 3클래스만 쓰고 싶다면 SensiML에서 재 export 후 펌웨어 라벨 테이블도 같이 맞추면 됨.

---

## 3. 빌드 / 링크 (`libsensiml.a`)

- `kb_model_init`, `kb_run_model`, `kb_reset_model` 등은 **`libsensiml.a`**에 들어 있음.
- MPLAB X가 `Makefile-default.mk`를 다시 만들 때 **`linkerLibFileItem`만으로는 `.a`가 빠지는 경우**가 있음(특히 파일이 없을 때 재생성한 적이 있으면).
- 대응: `firmware/src/sensiml/lib/libsensiml.a` 실물 유지, 링커에 라이브러리 추가, 필요 시 **XC32 링커 Additional options**에 `.a` 경로 추가 등.
- **헤더(`kb.h`, `MAX_VECTOR_SIZE` 등) + `libsensiml.a`는 항상 같은 Knowledge Pack export에서 가져오는 것**이 안전함.

---

## 4. `main.c` — 입력·캡처 개념

### 4.1 `ML_INPUT_MODE`

- `0` 델타, `1` signal, `2` reference, `3` pseudo-signal(델타 클램프 후 baseline 오프셋) 등.
- **학습 CSV가 무엇이었는지**와 맞추는 것이 최우선.

### 4.2 터치 캡처 FSM (요지)

- **IDLE → CAPTURE → HOLD**
- `CAPTURE`에서 `measurement_done_touch`마다 `sml_recognition_run` → 내부적으로 **`kb_run_model` 1스텝**.
- N스텝 후 **한 번** 요약 라벨 출력(`last_valid_class` 또는 `none`).
- HOLD에서는 손 뗄 때까지 재캡처 방지.

### 4.3 `CAPTURE_NUM_SAMPLES` (예: 15 vs 20)

- **고정으로 20이 “필수”는 아님.** 다만 학습이 **길이 20 윈도우**로 맞춰져 있으면 **20에 맞추는 것이 합리적**.
- 너무 짧으면 세그먼트/피처가 채워지기 전에 끝나 **`ret`이 -1만 나오고 `none`이 잦을 수 있음.**

### 4.4 터치 감지는 **델타(|signal−ref|)** 기준

- `touch_start` / `touch_active`는 **`abs_d0`, `abs_d1`**(델타 절댓값)와 임계값 비교.
- “ref가 509인데 signal이 513일 때” 같은 **절대값 비교가 아님**; **|Δ| ≥ START**일 때 시작.

### 4.5 `sample[2]` / `ns_use`

- `kb_run_model(sample, nsensors, …)`에서 **한 시각의 벡터**(ch0, ch1…)를 넣음. **두 채널을 더해서 넣지는 않음.**
- `kb_get_num_sensor_buffers`로 나온 **`expected_nsensors`가 1이면 사실상 ch0만 사용**에 가깝고, 2면 2열 입력.

---

## 5. 학습 데이터 방법론 (문서 기준 정리)

- **두 채널** UART로 수집, 클래스 예: finger / noise / water.
- **각 클래스마다** 편집 시 **delta가 threshold(20)를 넘기 전·직후를 기준으로 한 윈도우**에 **signal 20포인트**, 샘플링 **약 4 ms**라고 한 경우가 있음.
- 해석상 **“|델타| ≥ 20이 되는 그 샘플을 윈도우 끝(또는 기준점)으로 두고, 그 앞 19 + 해당 1 = 20”**에 가깝게 맞추면 학습과 시간 정렬이 잘 맞음.

### 5.1 현재 펌웨어(작은 START 임계값)와의 괴리

- 펌웨어에서 **START를 3**처럼 낮게 잡으면, 학습은 **20 근처 크로스** 기준인데 **훨씬 이른 시점**에서 20스텝을 밀어 넣게 됨 → **위상 불일치** 가능.

### 5.2 개선 아이디어 (링/시프트 버퍼)

- **항상 최근 20샘플**을 갱신하다가, **|델타| ≥ 20**인 순간 **그때의 20포인트(직전 19 + 크로스 1 등, 정의에 맞게)** 를 고정.
- 그 후 `kb_flush_model_buffer` 하고 **그 20스텝을 시간 순서대로** `kb_run_model`에 넣기.

---

## 6. 배열을 “한 번에” 모델에 넣을 수 있나?

### 6.1 `kb_run_model`

- **한 호출 = 한 time step**(채널 수만큼 `int16_t`).
- **길이 20 배열 포인터 하나를 한 번에** 넘기는 형태는 **아님.**

### 6.2 실무 패턴

- 배열에 20스텝 저장 → **`kb_flush` 후 for 루프로 20번 `kb_run_model`** → 스트리밍과 동일한 시계열 재생.

### 6.3 청크 API (있을 때)

`kb.h`에 따르면:

- **`kb_add_segment(uint16_t *pBuffer, int32_t len, int32_t nbuffs, int32_t model_index)`** — 사용자 버퍼로 링 설정.
- **`kb_run_segment(int32_t model_index)`** — 청크 실행.

주의: 문서상 **`kb_run_segment`는 Sensor Transform / Sensor Filter를 건너뛸 수 있음.** 파이프라인과 export에 맞는지 확인 없이 쓰기 어렵고, **가장 단순한 경로는 여전히 `kb_run_model` 20회**인 경우가 많음.

### 6.4 기타 저수준 API

- `kb_data_streaming`, `kb_segmentation`, `kb_feature_generation` 등 — 고급/분해 실행용.

---

## 7. 참고 파일 (저장소 내)

- `KB_API_REFERENCE.md` — `kb_*` 요약 참고.

---

## 8. 체크리스트 (실기 동작 맞출 때)

1. [ ] 학습에 쓴 것과 동일한 **물리량**(signal / delta / 전처리)과 `ML_INPUT_MODE` 일치  
2. [ ] **윈도우 길이**(20)와 **크로스 임계값**(20)을 학습과 맞출지, 캡처 시작 조건 조정  
3. [ ] `expected_nsensors`와 실제 `sample[]` 채널 수 일치  
4. [ ] `libsensiml.a` + `kb.h` / `model_json` 동일 export  
5. [ ] 링크 줄에 `libsensiml.a` 포함 여부(빌드 로그 확인)

---

*이 문서는 대화 기반 정리이며, 최종 동작은 사용 중인 Knowledge Pack 문서·SensiML export 메타데이터를 우선합니다.*
