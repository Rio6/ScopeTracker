#![feature(asm)]

#![no_std]
#![no_main]

extern crate ruduino;

use ruduino::{
    Pin, Register, RegisterBits,
    config::CPU_FREQUENCY_HZ,
    legacy::serial,
    cores::atmega328::*
};

#[no_mangle]
pub extern fn main() -> ! {

    const UBRR: u16 = (CPU_FREQUENCY_HZ / 16 / 9600 - 1) as u16;

    serial::Serial::new(UBRR)
        .character_size(serial::CharacterSize::EightBits)
        .mode(serial::Mode::Asynchronous)
        .parity(serial::Parity::Disabled)
        .stop_bits(serial::StopBits::OneBit)
        .configure();

    port::C0::set_input();
    adc_init();

    for c in "Hello, world\n".bytes() {
        serial::transmit(c);
    }

    loop {
        let read = adc_read(0);
        for bit in (0..10).rev() {
            serial::transmit(
                if (read & (1 << bit)) > 0 {
                    '1' as u8
                } else {
                    '0' as u8
                }
            );
        }

        serial::transmit(b'\n');
        delay();
    }
}

fn delay() {
    for _ in 0..10000 {
        unsafe { asm!("" :::: "volatile")}
    }
}

fn adc_init() {
    // Setup ADC with AVCC ref
    ADMUX::write(ADMUX::REFS0);
    // Enable ADC with prescaling factor of 128
    ADCSRA::write(ADCSRA::ADEN | RegisterBits::new(0b111));
}

fn adc_read(pin: u8) -> u16 {
    // Set pin number
    ADMUX::set_mask_raw(pin & 0x7);

    // Start ADC
    ADCSRA::set(ADCSRA::ADSC);

    // Wait for ADC
    while ADCSRA::is_clear(ADCSRA::ADSC) {}

    // Read ADC
    return ADC::read();
}
