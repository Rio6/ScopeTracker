start_xargo_env() {
    OLD_PATH=$PATH
    OLD_PS1=$PS1

    basedir=$(dirname $(cargo locate-project | jq -r .root))
    export PATH="$PATH:$HOME/.platformio/packages/toolchain-atmelavr/bin/:$HOME/.cargo/bin:$basedir/tools"
    export AVRDUDE_PATH="$HOME/.platformio/packages/tool-avrdude"
    export XARGO_RUST_SRC="$HOME/.rustup/toolchains/avr-toolchain/lib/rustlib/src/rust/src"
    export RUST_TARGET_PATH="$basedir"
    export RUSTUP_TOOLCHAIN="avr-toolchain"
    export PS1="$(sed 's/\(.*\)\$\(.*\)/\1 [xargo-env]$\2/' <<< "$PS1")"
    export XARGO_ENV_SET=1

    echo "xargo environment activated"
}

stop_xargo_env() {

    PATH=$OLD_PATH
    PS1=$OLD_PS1

    unset XARGO_RUST_SRC
    unset RUST_TARGET_PATH
    unset RUSTUP_TOOLCHAIN
    unset XARGO_ENV_SET

    echo "xargo environment deactivated"
}

if [ -z "$XARGO_ENV_SET" ]; then
    start_xargo_env
    trap stop_xargo_env EXIT
else
    stop_xargo_env
fi
