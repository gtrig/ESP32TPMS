
#define LGFX_USE_V1
#include "Arduino.h"
#include "main.h"
#include <LovyanGFX.hpp>
#include <lvgl.h>
#include "BLEDevice.h"

#ifdef USE_UI
#include "ui/ui.h"
#endif

#define buf_size 10

BLEScan *pBLEScan;
BLEClient *pClient;

// Variables
static BLEAddress *pServerAddress;
// TPMS BLE SENSORS known addresses
String knownAddresses[] = {"81:ea:ca:22:d5:e7", "82:ea:ca:33:02:d7"};
String FrontAddress = "81:ea:ca:22:d5:e7";
String BackAddress = "82:ea:ca:33:02:d7";

class LGFX : public lgfx::LGFX_Device
{

  lgfx::Panel_GC9A01 _panel_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Touch_CST816S _touch_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();

      // SPIバスの設定
      cfg.spi_host = SPI; // 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
      // ※ ESP-IDFバージョンアップに伴い、VSPI_HOST , HSPI_HOSTの記述は非推奨になるため、エラーが出る場合は代わりにSPI2_HOST , SPI3_HOSTを使用してください。
      cfg.spi_mode = 0;                  // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 80000000;         // 传输时的SPI时钟（最高80MHz，四舍五入为80MHz除以整数得到的值）
      cfg.freq_read = 20000000;          // 接收时的SPI时钟
      cfg.spi_3wire = true;              // 受信をMOSIピンで行う場合はtrueを設定
      cfg.use_lock = true;               // 使用事务锁时设置为 true
      cfg.dma_channel = SPI_DMA_CH_AUTO; // 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
      // ※ ESP-IDFバージョンアップに伴い、DMAチャンネルはSPI_DMA_CH_AUTO(自動設定)が推奨になりました。1ch,2chの指定は非推奨になります。
      cfg.pin_sclk = SCLK; // SPIのSCLKピン番号を設定
      cfg.pin_mosi = MOSI; // SPIのCLKピン番号を設定
      cfg.pin_miso = MISO; // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_dc = DC;     // SPIのD/Cピン番号を設定  (-1 = disable)

      _bus_instance.config(cfg);              // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance); // バスをパネルにセットします。
    }

    {                                      // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config(); // 表示パネル設定用の構造体を取得します。

      cfg.pin_cs = CS;   // CSが接続されているピン番号   (-1 = disable)
      cfg.pin_rst = RST; // RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_busy = -1; // BUSYが接続されているピン番号 (-1 = disable)

      // ※ 以下の設定値はパネル毎に一般的な初期値が設定さ BUSYが接続されているピン番号 (-1 = disable)れていますので、不明な項目はコメントアウトして試してみてください。

      cfg.memory_width = WIDTH;   // ドライバICがサポートしている最大の幅
      cfg.memory_height = HEIGHT; // ドライバICがサポートしている最大の高さ
      cfg.panel_width = WIDTH;    // 実際に表示可能な幅
      cfg.panel_height = HEIGHT;  // 実際に表示可能な高さ
      cfg.offset_x = OFFSET_X;    // パネルのX方向オフセット量
      cfg.offset_y = OFFSET_Y;    // パネルのY方向オフセット量
      cfg.offset_rotation = 0;    // 值在旋转方向的偏移0~7（4~7是倒置的）
      cfg.dummy_read_pixel = 8;   // 在读取像素之前读取的虚拟位数
      cfg.dummy_read_bits = 1;    // 读取像素以外的数据之前的虚拟读取位数
      cfg.readable = false;       // 如果可以读取数据，则设置为 true
      cfg.invert = true;          // 如果面板的明暗反转，则设置为 true
      cfg.rgb_order = RGB_ORDER;  // 如果面板的红色和蓝色被交换，则设置为 true
      cfg.dlen_16bit = false;     // 对于以 16 位单位发送数据长度的面板，设置为 true
      cfg.bus_shared = false;     // 如果总线与 SD 卡共享，则设置为 true（使用 drawJpgFile 等执行总线控制）

      _panel_instance.config(cfg);
    }

    {                                      // Set backlight control. (delete if not necessary)
      auto cfg = _light_instance.config(); // Get the structure for backlight configuration.

      cfg.pin_bl = BL;     // pin number to which the backlight is connected
      cfg.invert = false;  // true to invert backlight brightness
      cfg.freq = 44100;    // backlight PWM frequency
      cfg.pwm_channel = 1; // PWM channel number to use

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance); // Sets the backlight to the panel.
    }

    { // タッチスクリーン制御の設定を行います。（必要なければ削除）
      auto cfg = _touch_instance.config();

      cfg.x_min = 0;        // タッチスクリーンから得られる最小のX値(生の値)
      cfg.x_max = 240;      // タッチスクリーンから得られる最大のX値(生の値)
      cfg.y_min = 0;        // タッチスクリーンから得られる最小のY値(生の値)
      cfg.y_max = 240;      // タッチスクリーンから得られる最大のY値(生の値)
      cfg.pin_int = TP_INT; // INTが接続されているピン番号
      // cfg.pin_rst = TP_RST;
      cfg.bus_shared = false;  // 画面と共通のバスを使用している場合 trueを設定
      cfg.offset_rotation = 0; // 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定
      cfg.i2c_port = 0;        // 使用するI2Cを選択 (0 or 1)
      cfg.i2c_addr = 0x15;     // I2Cデバイスアドレス番号
      cfg.pin_sda = I2C_SDA;   // SDAが接続されているピン番号
      cfg.pin_scl = I2C_SCL;   // SCLが接続されているピン番号
      cfg.freq = 400000;       // I2Cクロックを設定

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance); // タッチスクリーンをパネルにセットします。
    }

    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};

LGFX tft;

static const uint32_t screenWidth = WIDTH;
static const uint32_t screenHeight = HEIGHT;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[2][screenWidth * buf_size];

String getTimeString()
{
  unsigned long currentTime = millis();
  String hours = String(currentTime / 3600000);
  String minutes = String((currentTime / 60000) % 60);
  String seconds = String((currentTime / 1000) % 60);

  if (hours.length() == 1)
  {
    hours = "0" + hours;
  }
  if (minutes.length() == 1)
  {
    minutes = "0" + minutes;
  }

  if (seconds.length() == 1)
  {
    seconds = "0" + seconds;
  }
  return hours + ":" + minutes + ":" + seconds;
}

void updateFrontTyreValues(String battery, String pressure, String temperature)
{
  lv_label_set_text(ui_Front_Battery_Value, battery.c_str());
  lv_label_set_text(ui_Front_Pressure_Value, pressure.c_str());
  lv_label_set_text(ui_Front_Temperature_Value, temperature.c_str());
}

void updateBackTyreValues(String battery, String pressure, String temperature)
{
  lv_label_set_text(ui_Back_Battery_Value, battery.c_str());
  lv_label_set_text(ui_Back_Pressure_Value, pressure.c_str());
  lv_label_set_text(ui_Back_Temperature_Value, temperature.c_str());
}

void updateValues()
{
  static unsigned long lastTime = 0;
  unsigned long currentTime = millis();

  if (currentTime - lastTime > 1000)
  {
    lastTime = currentTime;
    lv_label_set_text(ui_Timer, getTimeString().c_str());
  }
}

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  if (tft.getStartCount() == 0)
  {
    tft.endWrite();
  }

  tft.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (lgfx::swap565_t *)&color_p->full);
  lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

/*Read the touchpad*/
void my_touchpad_read_2(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{

  bool touched;
  uint8_t gesture;
  uint16_t touchX, touchY;

  touched = tft.getTouch(&touchX, &touchY);

  if (!touched)
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

/* BLE functions */
static void notifyCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
  // Serial.print("Notify callback for characteristic ");
  // Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  // Serial.print(" of data length ");
  // Serial.println(length);
}

String retmanData(String txt, int shift)
{
  // Return only manufacturer data string
  int start = txt.indexOf("data: ") + 6 + shift;
  return txt.substring(start, start + (36 - shift));
}

byte retByte(String Data, int start)
{
  // Return a single byte from string
  int sp = (start) * 2;
  char *ptr;
  return strtoul(Data.substring(sp, sp + 2).c_str(), &ptr, 16);
}

long returnData(String Data, int start)
{
  // Return a long value with little endian conversion
  return retByte(Data, start) | retByte(Data, start + 1) << 8 | retByte(Data, start + 2) << 16 | retByte(Data, start + 3) << 24;
}

int returnBatt(String Data)
{
  // Return battery percentage
  return retByte(Data, 16);
}

int returnAlarm(String Data)
{
  // Return battery percentage
  return retByte(Data, 17);
}
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice Device)
  {
    // Serial.print("BLE Advertised Device found: ");
    // Serial.println(Device.toString().c_str());
    pServerAddress = new BLEAddress(Device.getAddress());
    bool known = false;
    bool Master = false;
    String ManufData = Device.toString().c_str();
    for (int i = 0; i < (sizeof(knownAddresses) / sizeof(knownAddresses[0])); i++)
    {
      if (strcmp(pServerAddress->toString().c_str(), knownAddresses[i].c_str()) == 0)
        known = true;
    }
    if (known)
    {
      String instring = retmanData(ManufData, 0);
      // Serial.println(instring);
      // Serial.print("Device found: ");
      // Serial.println(Device.getRSSI());
      // // Tire Temperature in C°
      // Serial.print("Temperature: ");
      // Serial.print(returnData(instring, 12) / 100.0);
      // Serial.println("C°");
      // // Tire pressure in Kpa
      // Serial.print("Pressure:    ");
      // Serial.print(returnData(instring, 8) / 1000.0);
      // Serial.println("Kpa");
      // // Tire pressure in Bar
      // Serial.print("Pressure:    ");
      // Serial.print(returnData(instring, 8) / 100000.0);
      // Serial.println("bar");
      // // Tire pressure in PSI
      // Serial.print("Pressure:    ");
      // Serial.print(returnData(instring, 8) / 6894.76);
      // // Battery percentage
      // Serial.print("Battery:     ");
      // Serial.print(returnBatt(instring));
      // Serial.println("%");
      // if (returnAlarm(instring))
      // {
      //   Serial.println("ALARM!");
      // }
      // Serial.println("");

      if (strcmp(pServerAddress->toString().c_str(), FrontAddress.c_str()) == 0)
      {
        updateFrontTyreValues(String(returnBatt(instring)), String(returnData(instring, 8) / 6894.76), String(returnData(instring, 12) / 100.0));
      }
      else if (strcmp(pServerAddress->toString().c_str(), BackAddress.c_str()) == 0)
      {
        updateBackTyreValues(String(returnBatt(instring)), String(returnData(instring, 8) / 6894.76), String(returnData(instring, 12) / 100.0));
      }
      Device.getScan()->stop();
      delay(100);
    }
  }
};

void setup()
{
  Serial.begin(115200); /* prepare for possible serial debug */

  Serial.println("Starting up device");

  Serial.print("Init BLE. ");
  BLEDevice::init("");
  pClient = BLEDevice::createClient();
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  Serial.println("Done");

  tft.init();
  tft.initDMA();
  tft.startWrite();
  tft.fillScreen(TFT_BLACK);

  lv_init();

  lv_disp_draw_buf_init(&draw_buf, buf[0], buf[1], screenWidth * buf_size);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read_2;
  lv_indev_drv_register(&indev_drv);

#ifdef USE_UI
  ui_init();
#else
  lv_obj_t *label1 = lv_label_create(lv_scr_act());
  lv_obj_align(label1, LV_ALIGN_TOP_MID, 0, 30);
  lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(label1, screenWidth - 50);
  lv_label_set_text(label1, "Hello there! You have not included UI files, add you UI files and "
                            "uncomment this line\n'//#define USE_UI' in include/main.h\n"
                            "You should be able to move the slider below");

  /*Create a slider below the label*/
  lv_obj_t *slider1 = lv_slider_create(lv_scr_act());
  lv_obj_set_width(slider1, screenWidth - 60);
  lv_obj_align_to(slider1, label1, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
#endif
  tft.setBrightness(200);
}

void loop()
{

  lv_timer_handler();
  delay(1);
  BLEScanResults scanResults = pBLEScan->start(5);
  Serial.println("Looping");
  updateValues();
}
