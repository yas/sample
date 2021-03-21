#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ...
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

static const char* dev_name = "/dev/i2c-1";

/*! I2Cスレーブデバイスからデータを読み込む.
 * @param[in] dev_addr デバイスアドレス.
 * @param[in] reg_addr レジスタアドレス.
 * @param[out] data 読み込むデータの格納場所を指すポインタ.
 * @param[in] length 読み込むデータの長さ.
 */
int8_t i2c_read(
    uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint16_t length) {
  /* I2Cデバイスをオープンする. */
  int32_t fd = open(dev_name, O_RDWR);
  if (fd == -1) {
    fprintf(stderr, "i2c_read: failed to open: %s\n", strerror(errno));
    return -1;
  }

  /* I2C-Readメッセージを作成する. */
  struct i2c_msg messages[] = {
      { dev_addr, 0, 1, &reg_addr },         /* レジスタアドレスをセット. */
      { dev_addr, I2C_M_RD, length, data },  /* dataにlengthバイト読み込む. */
  };
  struct i2c_rdwr_ioctl_data ioctl_data = { messages, 2 };

  /* I2C-Readを行う. */
  if (ioctl(fd, I2C_RDWR, &ioctl_data) != 2) {
    fprintf(stderr, "i2c_read: failed to ioctl: %s\n", strerror(errno));
    close(fd);
    return -1;
  }

  close(fd);
  return 0;
}

/*! I2Cスレーブデバイスにデータを書き込む.
 * @param[in] dev_addr デバイスアドレス.
 * @param[in] reg_addr レジスタアドレス.
 * @param[in] data 書き込むデータの格納場所を指すポインタ.
 * @param[in] length 書き込むデータの長さ.
 */
int8_t i2c_write(
    uint8_t dev_addr, uint8_t reg_addr, const uint8_t* data, uint16_t length) {
  /* I2Cデバイスをオープンする. */
  int32_t fd = open(dev_name, O_RDWR);
  if (fd == -1) {
    fprintf(stderr, "i2c_write: failed to open: %s\n", strerror(errno));
    return -1;
  }

  /* I2C-Write用のバッファを準備する. */
  uint8_t* buffer = (uint8_t*)malloc(length + 1);
  if (buffer == NULL) {
    fprintf(stderr, "i2c_write: failed to memory allocate\n");
    close(fd);
    return -1;
  }
  buffer[0] = reg_addr;              /* 1バイト目にレジスタアドレスをセット. */
  memcpy(&buffer[1], data, length);  /* 2バイト目以降にデータをセット. */

  /* I2C-Writeメッセージを作成する. */
  struct i2c_msg message = { dev_addr, 0, (unsigned short int)(length + 1), buffer };
  struct i2c_rdwr_ioctl_data ioctl_data = { &message, 1 };

  /* I2C-Writeを行う. */
  if (ioctl(fd, I2C_RDWR, &ioctl_data) != 1) {
    fprintf(stderr, "i2c_write: failed to ioctl: %s\n", strerror(errno));
    free(buffer);
    close(fd);
    return -1;
  }

  free(buffer);
  close(fd);
  return 0;
}
