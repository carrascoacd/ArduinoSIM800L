#ifndef __ARDUCAM_H__
#define __ARDUCAM_H__

#include <Wire.h>
#include <SPI.h>
#include <ArduCAM.h>
#include <SD.h>

#define FRAMES_NUM 0x00
#define CAM_CS 5

const char IMG_NAME[] PROGMEM = "%d.jpg";

void buildImageName(char *filename, uint8_t currentImage) {
  sprintf_P(filename, IMG_NAME, currentImage);
}

bool takePicture(const char *imageName)
{
  pinMode(CAM_CS, OUTPUT);
  digitalWrite(CAM_CS, HIGH);

  ArduCAM myCAM(OV2640, CAM_CS);

  //Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100);

  //Change to JPEG capture mode and initialize the OV5640 module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.clear_fifo_flag();
  myCAM.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);

  // Low power mode OFF
  myCAM.clear_bit(ARDUCHIP_GPIO, GPIO_PWDN_MASK);

  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  myCAM.OV2640_set_JPEG_size(OV2640_1600x1200);

  //Start capture
  myCAM.start_capture();
  info(F("Start capture"));

  uint8_t temp = 0, temp_last = 0;
  uint32_t length = 0;
  int i = 0;
  File outFile;
  uint8_t bufSize = 8;
  byte buf[bufSize];
  bool isHeader = false;
  bool imageTook = false;

  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK) && i < 10)
  {
    ++i;
    delay(500);
  }
  i = 0;

  length = myCAM.read_fifo_length();
  info(F("The fifo length is:"), FALSE);
  info(length, TRUE);

  if (length >= MAX_FIFO_SIZE) //8M
  {
    info("Over size.");
    return 0;
  }
  if (length == 0) //0 kb
  {
    info(F("Size is 0."));
    return 0;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  i = 0;
  while (length--)
  {
    temp_last = temp;
    temp = SPI.transfer(0x00);
    //Read JPEG data from FIFO
    if ((temp == 0xD9) && (temp_last == 0xFF)) //If find the end, break while,
    {
      buf[i++] = temp; //save the last  0XD9
      //Write the remain bytes in the buffer
      myCAM.CS_HIGH();
      outFile.write(buf, i);
      //Close the file
      outFile.close();
      info(F("Capture Done!!"));

      // Low power mode ON
      myCAM.set_bit(ARDUCHIP_GPIO, GPIO_PWDN_MASK);
      
      isHeader = false;
      myCAM.CS_LOW();
      myCAM.set_fifo_burst();
      i = 0;
      imageTook = TRUE;
    }
    if (isHeader == true)
    {
      //Write image data to buffer if not full
      if (i < bufSize)
        buf[i++] = temp;
      else
      {
        //Write bufSize bytes image data to file
        myCAM.CS_HIGH();
        outFile.write(buf, bufSize);
        i = 0;
        buf[i++] = temp;
        myCAM.CS_LOW();
        myCAM.set_fifo_burst();
      }
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      isHeader = true;
      myCAM.CS_HIGH();

      outFile = SD.open(imageName, O_WRITE | O_CREAT | O_TRUNC);
      if (!outFile)
      {
        info(F("CAM. File open failed: "), FALSE);
        info(imageName);

        // Low power mode ON
        myCAM.set_bit(ARDUCHIP_GPIO, GPIO_PWDN_MASK);
        return imageTook;
      }
      myCAM.CS_LOW();
      myCAM.set_fifo_burst();
      buf[i++] = temp_last;
      buf[i++] = temp;
    }
  }

  myCAM.CS_HIGH();
  //Clear the capture done flag
  myCAM.clear_fifo_flag();

  pinMode(CAM_CS, INPUT);

  return imageTook;
}

#endif __ARDUCAM_H__
