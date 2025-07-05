#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>

typedef struct _VideoInfo{
  const uint8_t* const* frames;
  const uint16_t* frames_size;
  uint16_t num_frames;
} VideoInfo;

#include "video01.h"
#include "video02.h"
#include "video03.h"
#include "video04.h"
//#include "video05.h"
#include "video06.h"
#include "video07.h"
#include "video08.h"
#include "video09.h"
#include "video10.h"
#include "video11.h"
#include "video12.h"
#include "video13.h"
#include "video14.h"

TFT_eSPI tft = TFT_eSPI();

// Khai báo danh sách các video
VideoInfo* videoList[] = {
  &video01,
  &video02,
  &video03,
  &video04,
 // &video05,
  &video06,
  &video07,
  &video08,
  &video09,
  &video10,
  &video11,
  &video12,
  &video13,
  &video14,
};
const uint8_t NUM_VIDEOS = sizeof(videoList) / sizeof(videoList[0]);

// Biến điều khiển
uint8_t currentVideoIndex = 0;
uint16_t currentFrameIndex = 0;
unsigned long lastFrameTime = 0;
unsigned long lastButtonTime = 0;
const uint16_t FRAME_DELAY = 50; // ms giữa các frame
const uint16_t BUTTON_DEBOUNCE = 200; // ms debounce cho nút

// Chân nút cảm ứng
const uint8_t TOUCH_PIN = 13;
bool lastButtonState = HIGH;

// Tính toán vị trí căn giữa cho video 160x80 trên màn hình 160x128
const int16_t VIDEO_WIDTH = 160;
const int16_t VIDEO_HEIGHT = 80;
const int16_t SCREEN_WIDTH = 160;
const int16_t SCREEN_HEIGHT = 128;
const int16_t OFFSET_X = (SCREEN_WIDTH - VIDEO_WIDTH) / 2;
const int16_t OFFSET_Y = (SCREEN_HEIGHT - VIDEO_HEIGHT) / 2;

// Callback để TJpg_Decoder gọi khi vẽ block ảnh
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  // Điều chỉnh vị trí để căn giữa
  int16_t adj_x = x + OFFSET_X;
  int16_t adj_y = y + OFFSET_Y;
  
  if (adj_x >= tft.width() || adj_y >= tft.height()) return false;
  
  // Cắt phần ảnh vượt ra ngoài màn hình
  if (adj_x + w > tft.width()) w = tft.width() - adj_x;
  if (adj_y + h > tft.height()) h = tft.height() - adj_y;
  
  tft.pushImage(adj_x, adj_y, w, h, bitmap);
  return true;
}

// Vẽ một frame của video cụ thể
void drawJPEGFrame(const VideoInfo* video, uint16_t frameIndex) {
  const uint8_t* jpg_data = (const uint8_t*)pgm_read_ptr(&video->frames[frameIndex]);
  uint16_t jpg_size = pgm_read_word(&video->frames_size[frameIndex]);
  
  if (!TJpgDec.drawJpg(0, 0, jpg_data, jpg_size)) {
    Serial.printf("❌ Decode failed on frame %d\n", frameIndex);
  }
}

// Chuyển sang video tiếp theo
void nextVideo() {
  currentVideoIndex = (currentVideoIndex + 1) % NUM_VIDEOS;
  currentFrameIndex = 0;
  
  // Xóa màn hình với màu đen để chuyển video mượt mà
  tft.fillScreen(TFT_BLACK);
  
  Serial.printf("📹 Switching to video %d\n", currentVideoIndex + 1);
}

// Kiểm tra nút cảm ứng
void checkButton() {
  bool currentButtonState = digitalRead(TOUCH_PIN);
  unsigned long currentTime = millis();
  
  // Phát hiện nhấn nút (từ HIGH xuống LOW) với debounce
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    if (currentTime - lastButtonTime > BUTTON_DEBOUNCE) {
      nextVideo();
      lastButtonTime = currentTime;
    }
  }
  
  lastButtonState = currentButtonState;
}

void setup() {
  Serial.begin(115200);
  
  // Cấu hình nút cảm ứng
  pinMode(TOUCH_PIN, INPUT_PULLUP);
  
  // Cấu hình TFT
  tft.begin();
  tft.setRotation(1); // Landscape mode
  tft.fillScreen(TFT_BLACK);
  
  // Vẽ khung viền để hiển thị vùng video
  tft.drawRect(OFFSET_X - 1, OFFSET_Y - 1, VIDEO_WIDTH + 2, VIDEO_HEIGHT + 2, TFT_WHITE);
  
  // Cấu hình JPEG decoder
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);
  
  Serial.println("🎬 Video Player Started");
  Serial.printf("📺 Screen: %dx%d, Video: %dx%d\n", SCREEN_WIDTH, SCREEN_HEIGHT, VIDEO_WIDTH, VIDEO_HEIGHT);
  Serial.printf("📍 Video offset: (%d, %d)\n", OFFSET_X, OFFSET_Y);
}

void loop() {
  unsigned long currentTime = millis();
  
  // Kiểm tra nút cảm ứng
  checkButton();
  
  // Kiểm tra xem có đến lúc vẽ frame tiếp theo không
  if (currentTime - lastFrameTime >= FRAME_DELAY) {
    VideoInfo* currentVideo = videoList[currentVideoIndex];
    
    // Vẽ frame hiện tại
    drawJPEGFrame(currentVideo, currentFrameIndex);
    
    // Chuyển sang frame tiếp theo
    currentFrameIndex++;
    
    // Nếu hết frame trong video hiện tại, chuyển sang video tiếp theo
    if (currentFrameIndex >= currentVideo->num_frames) {
      delay(500); // Tạm dừng giữa các video
      nextVideo();
    }
    
    lastFrameTime = currentTime;
  }
  
  // Delay nhỏ để tránh CPU quá tải
  delay(1);
}