SAM_PIN
=======
-- Add to .bashrc
alias pin="/your/path/here/pin-2.13-62732-gcc.4.4.7-linux/pin"
export PIN_ROOT=/your/path/here/pin-2.13-62732-gcc.4.4.7-linux

-- Compile

make obj-intel64/inscount.so


-- Run


pin -t obj-intel64/inscount.so -o inscount.log -- /bin/ls

