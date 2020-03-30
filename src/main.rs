#![feature(asm)]

#![no_std]
#![no_main]

extern crate ruduino;
mod adc;
mod printer;

use adc::{adc_init, adc_read};
use printer::*;
use ruduino::{Pin, cores::atmega328::*};

#[no_mangle]
pub extern fn main() -> ! {
    print_init(9600);
    adc_init();

    port::C0::set_input();

    prints("Hello, world\n");

    loop {
        let read = adc_read(0);
        //printb(read as u32, 16);
        printd(read as i32, 5);
        printc('\n');
        delay();
    }
}

fn delay() {
    for _ in 0..10000 {
        unsafe { asm!("" :::: "volatile")}
    }
}
