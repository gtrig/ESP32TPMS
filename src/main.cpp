
#define LGFX_USE_V1
#include "Arduino.h"
#include "main.h"
#include <LovyanGFX.hpp>
#include <lvgl.h>
#include "functions/tpms.h"
#include "functions/ble.h"

#ifdef USE_UI
#include "ui/ui.h"
#endif

#define buf_size 10



float FrontPressure = 35.123;
float FrontTemperature = 30.234;
float BackPressure = 42.123;
float BackTemperature = 30.234;

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

      cfg.spi_host = SPI; 
      
      cfg.spi_mode = 0;                  
      cfg.freq_write = 80000000;         
      cfg.freq_read = 20000000;          
      cfg.spi_3wire = true;              
      cfg.use_lock = true;               
      cfg.dma_channel = SPI_DMA_CH_AUTO; 
      
      cfg.pin_sclk = SCLK; 
      cfg.pin_mosi = MOSI; 
      cfg.pin_miso = MISO; 
      cfg.pin_dc = DC;     

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs = CS;
      cfg.pin_rst = RST; 
      cfg.pin_busy = -1; 

      cfg.memory_width = WIDTH;
      cfg.memory_height = HEIGHT;
      cfg.panel_width = WIDTH;
      cfg.panel_height = HEIGHT;
      cfg.offset_x = OFFSET_X;
      cfg.offset_y = OFFSET_Y;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = false;
      cfg.invert = true;
      cfg.rgb_order = RGB_ORDER;
      cfg.dlen_16bit = false;
      cfg.bus_shared = false;

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

    {
      auto cfg = _touch_instance.config();

      cfg.x_min = 0;        
      cfg.x_max = 240;      
      cfg.y_min = 0;        
      cfg.y_max = 240;      
      cfg.pin_int = TP_INT; 
      // cfg.pin_rst = TP_RST;
      cfg.bus_shared = false;
      cfg.offset_rotation = 0;
      cfg.i2c_port = 0;
      cfg.i2c_addr = 0x15;
      cfg.pin_sda = I2C_SDA;
      cfg.pin_scl = I2C_SCL;
      cfg.freq = 400000;

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
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

void setup()
{
  Serial.begin(115200); /* prepare for possible serial debug */
  Serial.println("Starting up device");
  bleInit();
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
  tft.setRotation(3);
}

void loop()
{

  lv_timer_handler();
  bleScan();
  Serial.println("Looping");
  updateValues();

  // updateFrontTyreValues(90, FrontPressure++, FrontTemperature++);
  // updateBackTyreValues(90, BackPressure++, BackTemperature++);
}
