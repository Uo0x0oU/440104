/*
  りずみかるししおどし (Rhythmical Shishi-odoshi) - 人感センサーver / 100度スイング
  ------------------------------------------------------------
  人感センサー(HC-SR501)が人を検知すると、サーボが筒(つつ)を
  持ち上げ→振り下ろす動作を、グリーグ「ペール・ギュント組曲第1番 Op.46
  第1曲『朝』」冒頭フレーズのリズムに合わせて繰り返します。
  1回の「コン」で、サーボは 100度 振り下ろします。

  ■ サーボ
    一般的な180度サーボ(SG90 / MG90S / MG996R など)で動作します。
    可動範囲の中央(90度)を挟むように 40度〜140度 を使い、
    両端に40度ずつ余裕を残しているので、端で唸りにくい設定です。

  ■ 人感センサー HC-SR501
    人を検知すると出力が HIGH になります。
    電源投入直後は出力が安定しないので、起動時に待ち時間を入れています。

  配線:
    サーボ信号線      -> Arduino D9
    サーボ+(赤)      -> 5V (MG996Rなど大きめのサーボは外部5V電源+コンデンサ推奨)
    サーボ-(茶/黒)    -> GND (外部電源を使う場合はArduinoのGNDとも共通にする)
    HC-SR501 OUT     -> Arduino D3
    HC-SR501 VCC     -> 5V
    HC-SR501 GND     -> GND

  ※ 機構の取り付け向きに合わせて、ANGLE_COCKED / ANGLE_STRIKE の
    差が100度になるようにずらして調整してください(例: 0と100、80と180)。
*/

#include <Servo.h>

#define SENSOR_PIN 3   // HC-SR501 の出力

Servo shishiServo;

const int SERVO_PIN = 9;

// --- サーボ角度(差が100度) ---
const int ANGLE_COCKED = 40;    // 尻尾(短い方)を持ち上げた状態=「ぎー」
const int ANGLE_STRIKE = 140;   // 尻尾が石(受け台)を打つ状態=「コン」  (140-40 = 100度)

// --- 動作時間(実機に合わせて要調整) ---
// 100度動くのにかかる時間。SG90で約0.17秒、MG996Rで約0.28秒が目安。
const int SWING_MS = 500;   // 100度動くのを待つ時間
const int HOLD_MS  = 100;   // 打った姿勢を保持する時間

// --- 人感センサー関連 ---
const unsigned long SENSOR_WARMUP_MS = 30000; // 起動直後のセンサー安定待ち(約30秒)

// --- テンポ(1ユニットの長さ ms) ---
// 「振り下ろし + 保持 + 戻り」が1ユニットに収まるよう、
// unitMs は 2*SWING_MS + HOLD_MS 以上にすること。
int unitMs = 2000;

// 「コンコンコンコン ココ ココーコン コンコンコンコン コーコンコーコン」
// 1音ごとの長さ(ユニット数)。各「コン」の開始間隔 = rhythm[i] * unitMs
int rhythm[] = {
  1, 1, 1, 1,      // コンコンコンコン
  1, 1,            // ココ
  1, 1, 2,         // ココーコン
  1, 1, 1, 1,      // コンコンコンコン
  2, 1, 2, 1       // コーコン コーコン
};
const int RHYTHM_LEN = sizeof(rhythm) / sizeof(rhythm[0]);

void setup() {
  Serial.begin(9600);
  pinMode(SENSOR_PIN, INPUT);

  shishiServo.write(ANGLE_COCKED);  // 先に初期位置を設定してから
  shishiServo.attach(SERVO_PIN);    // 接続(起動時の暴れ防止)
  delay(SWING_MS);

  // HC-SR501 は電源投入直後しばらく誤検知しやすいので待つ
  Serial.println("Sensor warming up...");
  delay(SENSOR_WARMUP_MS);
  Serial.println("Ready");
}

void loop() {
  int detected = digitalRead(SENSOR_PIN);
  Serial.println(detected);   // 動作確認用(1=検知, 0=未検知)

  if (detected == HIGH) {
    playSequence();

    // 人がいる間はセンサーが HIGH のままなので、
    // いったん LOW(人がいなくなる)に戻るまで待ってから次の検知を受け付ける
    while (digitalRead(SENSOR_PIN) == HIGH) {
      delay(200);
    }
  }

  delay(200);
}

// 1回の検知=フレーズ全体を再生
void playSequence() {
  for (int i = 0; i < RHYTHM_LEN; i++) {
    knock();  // ここで SWING_MS + HOLD_MS 経過する

    // 次の「コン」まで残りの時間を待つ(この間にサーボは構えの位置へ戻る)
    long waitMs = (long)rhythm[i] * unitMs - (SWING_MS + HOLD_MS);
    if (waitMs < SWING_MS) waitMs = SWING_MS; // 戻りきる前に次を打たないための保険
    delay(waitMs);
  }
  shishiServo.write(ANGLE_COCKED); // 最後は「構え」の姿勢に戻しておく
}

// 「コン」を1回鳴らす(100度振り下ろし→打った姿勢を保持→持ち上げ開始)
void knock() {
  shishiServo.write(ANGLE_STRIKE);  // 100度振り下ろす
  delay(SWING_MS);                  // 振り切るまで待つ(ここで石に当たる)
  delay(HOLD_MS);                   // 打った姿勢を少し保持
  shishiServo.write(ANGLE_COCKED);  // 構えの位置へ戻し始める(「ぎー」)
}
