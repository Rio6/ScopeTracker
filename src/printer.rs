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

pub fn printb(mut n: u32, width: i32) {
    n <<= 32 - width;
    for _ in 0..width {
        if n & 0x8000 > 0 {
            printc('1');
        } else {
            printc('0');
        }
        n <<= 1;
    }
}

pub fn printd(n: i32, width: i32) {
    let mut div = 10;
    for _ in 0..width {
        div *= 10;
    }

    for _ in 0..width {
        let c = '0' as u8 + (n / div % 10) as u8;
        serial::transmit(c);
        div /= 10;
    }
}
