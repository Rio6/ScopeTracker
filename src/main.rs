#![feature(asm)]

#![no_std]
#![no_main]

extern crate ruduino;

use ruduino::cores::atmega328::port;
use ruduino::Pin;

#[no_mangle]
pub extern fn main() {
    port::B5::set_output();

    loop {
        port::B5::set_high();
        small_delay();
        port::B5::set_low();
        small_delay();
    }
}

/// A small busy loop.
fn small_delay() {
    for _ in 0..1000 {
        unsafe { asm!("" :::: "volatile")}
    }
}
