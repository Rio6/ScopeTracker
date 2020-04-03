#![feature(asm, core_intrinsics)]

#![no_std]
#![no_main]

extern crate avr_libc;
extern crate ruduino;

mod adc;
mod printer;

use avr_libc::{M_PI, sin};
use adc::{adc_init, adc_read};
use printer::*;
use ruduino::{Pin, cores::atmega328::*};

#[no_mangle]
pub extern fn main() -> ! {
    print_init(9600);
    adc_init();

    port::C0::set_input();

    prints("Hello, world\n");

    unsafe {
        let x = sin(M_PI / 2.0) as f32;
        printf(x, 2, 10);
        printc('\n');
    }

    loop {
        /*
        let read = adc_read(0);
        printud(read);
        printc('\n');
        delay();
        */
    }
}

fn delay() {
    for _ in 0..10000 {
        unsafe { asm!("" :::: "volatile")}
    }
}
