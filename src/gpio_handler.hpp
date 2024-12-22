#pragma once
extern "C"
{
    #include <wiringPi.h>
}

#include <initializer_list>
#include <functional>

/// @brief Class to handle GPIOs
/// This class is used to handle GPIOs with a callback
/// The callback is called when the GPIO changes state away from the one it was initialized with
struct InitGpio
{
	int pin;
	int mode;
	enum class pull {off = 0, up, down};
	pull pull;
	std::function<void(void)> callback;
};

class GpioHandler
{
public:
    //TODO: Add a destructor to clean up the GPIOs
    /// @brief Constructor
    /// @param list_gpio List of GPIOs to initialize
    /// This can be used with an initializer list like this:
    /// GpioHandler gpio_handler({ {PIN_BUTTON, INPUT, InitGpio::pull::up, [](){media_player.next();}} });
	GpioHandler(std::initializer_list<InitGpio> list_gpio)
	{
		wiringPiSetupGpio();
		for(const auto& gpio : list_gpio)
		{
			pinMode(gpio.pin, gpio.mode);
			if (gpio.mode == INPUT)
			{
                auto isr_edge = INT_EDGE_BOTH;
				switch(gpio.pull)
				{
					case InitGpio::pull::off:
						pullUpDnControl(gpio.pin, PUD_OFF);
                        isr_edge = INT_EDGE_BOTH;
						break;
					case InitGpio::pull::up:
						pullUpDnControl(gpio.pin, PUD_UP);
                        isr_edge = INT_EDGE_FALLING;
						break;
					case InitGpio::pull::down:
						pullUpDnControl(gpio.pin, PUD_DOWN);
                        isr_edge = INT_EDGE_RISING;
						break;
				}
                wiringPiISR(gpio.pin, isr_edge, gpio.callback.target<void(void)>());
			}
		}
	}
};