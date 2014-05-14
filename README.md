SAM_PIN
=======
-- Add to .bashrc
alias pin="/your/path/here/pin-2.13-62732-gcc.4.4.7-linux/pin"
export PIN_ROOT=/your/path/here/pin-2.13-62732-gcc.4.4.7-linux

-- Compile

make


-- Run

make run

-or-

pin -t obj-intel64/sampin.so -o sampin.log -- /path/to/program/here

