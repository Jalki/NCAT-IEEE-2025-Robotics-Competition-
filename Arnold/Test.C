#include <wiringPi.h> // Include WiringPi library!

int main(void)
{
  // uses BCM numbering of the GPIOs and directly accesses the GPIO registers.
  wiringPiSetupGpio();

  // pin mode ..(INPUT, OUTPUT, PWM_OUTPUT, GPIO_CLOCK)
  // set pin 17 to input
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  // pull up/down mode (PUD_OFF, PUD_UP, PUD_DOWN) => down
  pullUpDnControl(2, PUD_DOWN);
  pullUpDnControl(3, PUD_DOWN);
  pinmo

  // get state of pin 17
  digitalWrite(2, 1);
  digitalWrite(3, 1);
}

//gcc -o W Test.C  -l wiringPi