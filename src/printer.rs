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

pub fn printb(mut n: u16, width: i16) {
    n <<= 16 - width;
    for _ in 0..width {
        if n & 0x8000 > 0 {
            printc('1');
        } else {
            printc('0');
        }
        n <<= 1;
    }
}

pub fn printud(mut n: u16) {
    const W: usize = 5;
    let mut buff: [u8; W] = [0; W];
    for i in 0..W {
        buff[i] = (n % 10) as u8;
        n /= 10;
    }
    for i in (0..W).rev() {
        printc(('0' as u8 + buff[i]) as char);
    }
}
