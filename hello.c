/*
  りずみかるししおどし (Rhythmical Shishi-odoshi) - ブザーなしver
  ------------------------------------------------------------
  ボタンを1回押すと、サーボが筒(つつ)を持ち上げ→振り下ろす動作を
  グリーグ「ペール・ギュント組曲第1番 Op.46 第1曲『朝』」冒頭フレーズの
  リズムに合わせて繰り返します。

  ブザーは使わず、筒の尻尾が実際に石(受け台)に当たる「コン」という
  物理的な音だけで鳴らす構成です。

  配線:
    サーボ信号線   -> Arduino D9
    サーボ+(赤)   -> 5V (電流が心配なら外部5V電源+コンデンサ推奨)
    サーボ-(茶/黒) -> GND
    ボタン        -> D2 と GND の間(内部プルアップ使用)

  ※ ANGLE_COCKED / ANGLE_STRIKE や unitMs は、実際に組み立てた
    機構のサイズ・重さ・「石」に当たる強さに合わせて必ず調整してください。
    機構だけで鳴らす場合、ANGLE_STRIKE をやや強め(振り下ろし速度が
    出る角度差)にしないと「コン」と聞こえる音量にならないことがあります。
*/

#include <Servo.h>

Servo shishiServo;

const int SERVO_PIN  = 9;
const int BUTTON_PIN = 2;

// --- サーボ角度(実機に合わせて要調整) ---
const int ANGLE_COCKED = 60;   // 尻尾(短い方)を持ち上げた状態=「ぎー」
const int ANGLE_STRIKE = 110;  // 尻尾が石(受け台)を打つ状態=「コン」

// --- テンポ(1ユニットの長さ ms。小さいほど速い) ---
int unitMs = 200;

// 「コンコンコンコン ココ ココーコン コンコンコンコン コーコンコーコン」
// を、1音ごとの長さ(ユニット数)の配列にしたもの。
// この配列の長さ分だけ knock() を呼び、その後 rhythm[i]*unitMs だけ待つ。
int rhythm[] = {
  1, 1, 1, 1,      // コンコンコンコン
  1, 1,            // ココ
  1, 1, 2,         // ココーコン
  1, 1, 1, 1,      // コンコンコンコン
  2, 1, 2, 1       // コーコン コーコン
};
const int RHYTHM_LEN = sizeof(rhythm) / sizeof(rhythm[0]);

void setup() {
  shishiServo.attach(SERVO_PIN);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  shishiServo.write(ANGLE_COCKED);
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
    knock();
    delay(rhythm[i] * unitMs);
  }
  shishiServo.write(ANGLE_COCKED); // 最後は「構え」の姿勢に戻しておく
}

// 「コン」を1回鳴らす(振り下ろし→打った姿勢を保持→ゆっくり持ち上げ)
void knock() {
  shishiServo.write(ANGLE_STRIKE);      // すばやく振り下ろす(ここで石に当たる)
  delay(120);                           // 打った姿勢を少し保持
  shishiServo.write(ANGLE_COCKED);      // ゆっくり持ち上げ直す(「ぎー」)
}
