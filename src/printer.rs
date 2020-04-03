use avr_libc::fmod;
use ruduino::{
    config::CPU_FREQUENCY_HZ,
    legacy::serial
};

pub fn print_init(baud: u32) {
    serial::Serial::new((CPU_FREQUENCY_HZ / 16 / baud - 1) as u16)
        .character_size(serial::CharacterSize::EightBits)
        .mode(serial::Mode::Asynchronous)
        .parity(serial::Parity::Disabled)
        .stop_bits(serial::StopBits::OneBit)
        .configure();
}

pub fn printc(c: char) {
    serial::transmit(c as u8);
}

pub fn prints(s: &str) {
    for c in s.chars() {
        printc(c);
    }
}

pub fn printb(mut n: u32, width: usize) {
    n <<= 32 - width;
    for _ in 0..width {
        if n & 0x80000000 > 0 {
            printc('1');
        } else {
            printc('0');
        }
        n <<= 1;
    }
}

pub fn printf(mut num: f32, before: usize, after: usize) {

    fn print_digit(n: f32) {
        unsafe {
            let x = fmod(n as f64, 10.0) as u8;
            serial::transmit('0' as u8 + x);
        }
    }

    if num < 0.0 {
        printc('-');
        num = -num;
    }

    for _ in 1..before {
        num /= 10.0;
    }

    for _ in 0..before {
        print_digit(num);
        num *= 10.0;
    }

    printc('.');

    for _ in 0..after {
        print_digit(num);
        num *= 10.0;
    }
}
