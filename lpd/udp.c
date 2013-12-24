/*
 * simple_example.c  
 *
 * Copyright 2012 Christopher De Vries
 * This program is distributed under the Artistic License 2.0, a copy of which
 * is included in the file LICENSE.txt
 */
#include "lpd8806led.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>


int main(int argc, char *argv[]) {
  int fd;              /* SPI device file descriptor */
  const int leds = 229; /* 50 LEDs in the strand */
  lpd8806_buffer buf;      /* Memory buffer for pixel values */
  int count;           /* Count of iterations (up to 3) */
  int i;               /* Counting Integer */
  unsigned char recv_buf[leds * 3];
  
    int sock;
    struct sockaddr_in myaddr;
    struct sockaddr_in remaddr;
    socklen_t addrlen = sizeof(remaddr);
    int recvlen;
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(9022);

    if (bind(sock, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
            perror("bind failed");
                return 0;
    }


  set_gamma(2.5,2.5,2.5);
  /* Open SPI device */
  fd = open("/dev/spidev0.0",O_WRONLY);
  if(fd<0) {
      /* Open failed */
      fprintf(stderr, "Error: SPI device open failed.\n");
      exit(1);
  }

  /* Initialize SPI bus for lpd8806 pixels */
  if(spi_init(fd)<0) {
      /* Initialization failed */
      fprintf(stderr, "Unable to initialize SPI bus.\n");
      exit(1);
  }

  /* Allocate memory for the pixel buffer and initialize it */
  if(lpd8806_init(&buf,leds)<0) {
      /* Memory allocation failed */
      fprintf(stderr, "Insufficient memory for pixel buffer.\n");
      exit(1);
  }

    for (;;) {
        recvlen = recvfrom(sock, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&remaddr, &addrlen);
        printf("%d\n", recvlen);
        for (count = 0; count < leds; ++count) {
            //write_color(
            write_gamma_color(
                &buf.pixels[count],
                recv_buf[(3 * count) + 0],
                recv_buf[(3 * count) + 1],
                recv_buf[(3 * count) + 2]
            );
        }
        send_buffer(fd,&buf);
    }

    memset(recv_buf, 0, sizeof(recv_buf));
    for (count = 0; count < leds; ++count) {
        recv_buf[count * 3] = 0xff;
    }

    for (count = 0; count < leds; ++count) {
        write_gamma_color(
            &buf.pixels[count],
            recv_buf[(3 * count) + 0],
            recv_buf[(3 * count) + 1],
            recv_buf[(3 * count) + 2]
        );
    }
    send_buffer(fd,&buf);

      usleep(1000000);
#if 0  /* Send the data to the lpd8806 lighting strand */
      if(send_buffer(fd,&buf)<0) {
        fprintf(stderr, "Error sending data.\n");
        exit(1);
      }

      /* Sleep for 1 second */
      usleep(1000000/60.0f);
    }
  }
#endif

for(i=0;i<leds;i++) {
    
      write_gamma_color(&buf.pixels[i],0x00,0x00,0x00);
      
  }
    send_buffer(fd,&buf);
  /* Although the program never gets to this point, below is how to clean up */

  /* Free the pixel buffer */
  lpd8806_free(&buf);

  /* Close the SPI device */
  close(fd);

  return 0;
}
