use ruduino::{
    Register,
    RegisterBits,
    cores::atmega328::*
};

pub fn adc_init() {
    // Setup ADC with AVCC ref
    ADMUX::write(ADMUX::REFS0);

    // Enable ADC with prescaling factor of 128
    ADCSRA::write(ADCSRA::ADEN | RegisterBits::new(0b111));
}

pub fn adc_read(pin: u8) -> u16 {
    // Set pin number
    ADMUX::set_mask_raw(pin & 0x7);

    // Start ADC
    ADCSRA::set(ADCSRA::ADSC);

    // Wait for ADC
    while ADCSRA::is_clear(ADCSRA::ADSC) {}

    // Read ADC
    return ADC::read();
}
