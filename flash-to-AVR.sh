#   Author Dillon Dickerson
#   arg ${1} is the filename without extension must be added as an arg
echo please note that if you are specifiing a specific filename NOT to include the extension
filename=${1:-"main"}
echo using: {${filename}} as basename
avr-gcc -g -Os -mmcu=atmega168 -c ${filename}.c
avr-gcc -mmcu=atmega168 -o ${filename}.elf ${filename}.o
avr-objcopy -j .text -j .data -O ihex ${filename}.elf ${filename}.flash.hex
avrdude -p m168 -c usbtiny -P usb -b 9600 -v -U flash:w:${filename}.flash.hex
# clean up
read -p "Do you want me to clean up the files? [y/n] " res
case "${res}" in
    [yY])
        rm ${filename}.o ${filename}.elf ${filename}.flash.hex
    ;;
    *)
        exit 0
    ;;
esac
