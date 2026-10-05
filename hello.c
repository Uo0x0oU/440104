/*
  りずみかるししおどし (Rhythmical Shishi-odoshi) - ブザーなし / 90度スイングver
  ------------------------------------------------------------
  ボタンを1回押すと、サーボが筒(つつ)を持ち上げ→振り下ろす動作を
  グリーグ「ペール・ギュント組曲第1番 Op.46 第1曲『朝』」冒頭フレーズの
  リズムに合わせて繰り返します。
  1回の「コン」で、サーボは 90度 振り下ろします。

  ■ サーボ
    一般的な180度サーボ(SG90 / MG90S / MG996R など)で動作します。
    可動範囲の中央付近(45度〜135度)を使うので、端で唸りにくい設定です。

  配線:
    サーボ信号線   -> Arduino D9
    サーボ+(赤)   -> 5V (MG996Rなど大きめのサーボは外部5V電源+コンデンサ推奨)
    サーボ-(茶/黒) -> GND (外部電源を使う場合はArduinoのGNDとも共通にする)
    ボタン        -> D2 と GND の間(内部プルアップ使用)

  ※ 機構の取り付け向きに合わせて、ANGLE_COCKED / ANGLE_STRIKE の
    差が90度になるようにずらして調整してください(例: 0と90、90と180)。
*/

#include <Servo.h>

Servo shishiServo;

const int SERVO_PIN  = 9;
const int BUTTON_PIN = 2;

// --- サーボ角度(差が90度) ---
const int ANGLE_COCKED = 45;    // 尻尾(短い方)を持ち上げた状態=「ぎー」
const int ANGLE_STRIKE = 135;   // 尻尾が石(受け台)を打つ状態=「コン」  (135-45 = 90度)

// --- 動作時間(実機に合わせて要調整) ---
// 90度動くのにかかる時間。SG90で約0.15秒、MG996Rで約0.25秒が目安。
const int SWING_MS = 500;   // 90度動くのを待つ時間
const int HOLD_MS  = 100;   // 打った姿勢を保持する時間

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
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  shishiServo.write(ANGLE_COCKED);  // 先に初期位置を設定してから
  shishiServo.attach(SERVO_PIN);    // 接続(起動時の暴れ防止)
  delay(SWING_MS);
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    playSequence();
    delay(500); // 連打防止(チャタリング対策も兼ねる)
  }
}

// ボタン1回分=フレーズ全体を再生
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

// 「コン」を1回鳴らす(90度振り下ろし→打った姿勢を保持→持ち上げ開始)
void knock() {
  shishiServo.write(ANGLE_STRIKE);  // 90度振り下ろす
  delay(SWING_MS);                  // 振り切るまで待つ(ここで石に当たる)
  delay(HOLD_MS);                   // 打った姿勢を少し保持
  shishiServo.write(ANGLE_COCKED);  // 構えの位置へ戻し始める(「ぎー」)
}
